/***************************************************************************
                                qgis.cpp

                             -------------------
    begin                : 2007
    copyright            : (C) 2007 by Gary E. Sherman
    email                : sherman@mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgis.h"
#ifndef QGSVERSION
#include "qgsversion.h"
#endif
#include <QCoreApplication>
#include <QColor>
#include <QDate>
#include <QFileInfo>
#include <QTime>
#include <QDateTime>
#include <QTextDocument>
#include "qgsconfig.h"
#include "qgslogger.h"
#include <quazip/quazipfile.h>
#include "qgswkbtypes.h"

#include <ogr_api.h>

// Version constants
//

// Version string
const char* QGis::QGIS_VERSION = VERSION;

// development version
const char* QGis::QGIS_DEV_VERSION = QGSVERSION;

// Version number used for comparing versions using the
// "Check QGIS Version" function
const int QGis::QGIS_VERSION_INT = VERSION_INT;

// Release name
const char* QGis::QGIS_RELEASE_NAME = RELEASE_NAME;

// Full release name
const char* QGis::QGIS_FULL_RELEASE_NAME = FULL_RELEASE_NAME;

// Build date
const char* QGis::QGIS_BUILD_DATE = BUILD_DATE;

#if GDAL_VERSION_NUM >= 1800
extern const CORE_EXPORT QString GEOPROJ4 = "+proj=longlat +datum=WGS84 +no_defs";
#else
extern const CORE_EXPORT QString GEOPROJ4 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
#endif

extern const CORE_EXPORT QString GEOWKT =
    "GEOGCS[\"WGS 84\", "
    "  DATUM[\"WGS_1984\", "
    "    SPHEROID[\"WGS 84\",6378137,298.257223563, "
    "      AUTHORITY[\"EPSG\",7030]], "
    "    TOWGS84[0,0,0,0,0,0,0], "
    "    AUTHORITY[\"EPSG\",6326]], "
    "  PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",8901]], "
    "  UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",9108]], "
    "  AXIS[\"Lat\",NORTH], "
    "  AXIS[\"Long\",EAST], "
    "  AUTHORITY[\"EPSG\",4326]]";

extern const CORE_EXPORT QString PROJECT_SCALES =
    "1:1000000,1:500000,1:250000,1:100000,1:50000,1:25000,"
    "1:10000,1:5000,1:2500,1:1000,1:500";

extern const CORE_EXPORT QString GEO_EPSG_CRS_AUTHID = "EPSG:4326";

extern const CORE_EXPORT QString GEO_NONE = "NONE";

const double QGis::DEFAULT_IDENTIFY_RADIUS = 0.5;
const double QGis::DEFAULT_SEARCH_RADIUS_MM = 2.;

//! Default threshold between map coordinates and device coordinates for map2pixel simplification
const float QGis::DEFAULT_MAPTOPIXEL_THRESHOLD = 1.0f;

const QColor QGis::DEFAULT_HIGHLIGHT_COLOR = QColor( 255, 0, 0, 128 );

double QGis::DEFAULT_HIGHLIGHT_BUFFER_MM = 0.5;

double QGis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM = 1.0;

// description strings for units
// Order must match enum indices
const char* QGis::qgisUnitTypes[] =
{
  QT_TRANSLATE_NOOP( "QGis::UnitType", "meters" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "feet" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "degrees" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "<unknown>" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "degrees" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "degrees" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "degrees" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "nautical miles" )
};

QGis::UnitType QGis::fromLiteral( QString literal, QGis::UnitType defaultType )
{
  for ( unsigned int i = 0; i < ( sizeof( qgisUnitTypes ) / sizeof( qgisUnitTypes[0] ) ); i++ )
  {
    if ( literal == qgisUnitTypes[ i ] )
    {
      return static_cast<UnitType>( i );
    }
  }
  return defaultType;
}

QString QGis::toLiteral( QGis::UnitType unit )
{
  return QString( qgisUnitTypes[ static_cast<int>( unit )] );
}

QString QGis::tr( QGis::UnitType unit )
{
  return QCoreApplication::translate( "QGis::UnitType", qPrintable( toLiteral( unit ) ) );
}

QGis::UnitType QGis::fromTr( QString literal, QGis::UnitType defaultType )
{
  for ( unsigned int i = 0; i < ( sizeof( qgisUnitTypes ) / sizeof( qgisUnitTypes[0] ) ); i++ )
  {
    if ( literal == QGis::tr( static_cast<UnitType>( i ) ) )
    {
      return static_cast<UnitType>( i );
    }
  }
  return defaultType;
}

double QGis::fromUnitToUnitFactor( QGis::UnitType fromUnit, QGis::UnitType toUnit )
{
#define DEGREE_TO_METER 111319.49079327358
#define FEET_TO_METER 0.3048
#define NMILE_TO_METER 1852.0

  // Unify degree units
  if ( fromUnit == QGis::DecimalDegrees || fromUnit == QGis::DegreesMinutesSeconds || fromUnit == QGis::DegreesDecimalMinutes )
    fromUnit = QGis::Degrees;
  if ( toUnit == QGis::DecimalDegrees || toUnit == QGis::DegreesMinutesSeconds || toUnit == QGis::DegreesDecimalMinutes )
    toUnit = QGis::Degrees;

  // Calculate the conversion factor between the specified units
  if ( fromUnit != toUnit && fromUnit != QGis::UnknownUnit && toUnit != QGis::UnknownUnit )
  {
    switch ( fromUnit )
    {
      case QGis::Meters:
      {
        if ( toUnit == QGis::Feet ) return 1.0 / FEET_TO_METER;
        if ( toUnit == QGis::Degrees ) return 1.0 / DEGREE_TO_METER;
        if ( toUnit == QGis::NauticalMiles ) return 1.0 / NMILE_TO_METER;
        break;
      }
      case QGis::Feet:
      {
        if ( toUnit == QGis::Meters ) return FEET_TO_METER;
        if ( toUnit == QGis::Degrees ) return FEET_TO_METER / DEGREE_TO_METER;
        if ( toUnit == QGis::NauticalMiles ) return FEET_TO_METER / NMILE_TO_METER;
        break;
      }
      case QGis::Degrees:
      {
        if ( toUnit == QGis::Meters ) return DEGREE_TO_METER;
        if ( toUnit == QGis::Feet ) return DEGREE_TO_METER / FEET_TO_METER;
        if ( toUnit == QGis::NauticalMiles ) return DEGREE_TO_METER / NMILE_TO_METER;
        break;
      }
      case QGis::NauticalMiles:
      {
        if ( toUnit == QGis::Meters ) return NMILE_TO_METER;
        if ( toUnit == QGis::Feet ) return NMILE_TO_METER / FEET_TO_METER;
        if ( toUnit == QGis::Degrees ) return NMILE_TO_METER / DEGREE_TO_METER;
        break;
      }
      case QGis::UnknownUnit:
        break;
    }
  }
  return 1.0;
}

bool QGis::addFileToZip( QuaZip* zip, QString filePath, QString zipFileName )
{
  if ( !zip )
  {
    return false;
  }

  QFileInfo fi( filePath );
  QuaZipFile zipLocalFile( zip );
  if ( !zipLocalFile.open( QIODevice::WriteOnly, QuaZipNewInfo( zipFileName ) ) )
  {
    return false;
  }

  //copy from local file to zip local file
  QFile localFile( filePath );
  if ( !localFile.open( QIODevice::ReadOnly ) )
  {
    return false;
  }

  QByteArray buffer;
  while ( !localFile.atEnd() )
  {
    buffer = localFile.read( 4096 );
    if ( !buffer.isEmpty() )
    {
      zipLocalFile.write( buffer );
    }
  }
  return true;
}

void *qgsMalloc( size_t size )
{
  if ( size == 0 || long( size ) < 0 )
  {
    QgsDebugMsg( QString( "Negative or zero size %1." ).arg( size ) );
    return NULL;
  }
  void *p = malloc( size );
  if ( p == NULL )
  {
    QgsDebugMsg( QString( "Allocation of %1 bytes failed." ).arg( size ) );
  }
  return p;
}

void *qgsCalloc( size_t nmemb, size_t size )
{
  if ( nmemb == 0 || long( nmemb ) < 0 || size == 0 || long( size ) < 0 )
  {
    QgsDebugMsg( QString( "Negative or zero nmemb %1 or size %2." ).arg( nmemb ).arg( size ) );
    return NULL;
  }
  void *p = qgsMalloc( nmemb * size );
  if ( p != NULL )
  {
    memset( p, 0, nmemb * size );
  }
  return p;
}

void qgsFree( void *ptr )
{
  free( ptr );
}

bool qgsVariantLessThan( const QVariant& lhs, const QVariant& rhs )
{
  switch ( lhs.type() )
  {
    case QVariant::Int:
      return lhs.toInt() < rhs.toInt();
    case QVariant::UInt:
      return lhs.toUInt() < rhs.toUInt();
    case QVariant::LongLong:
      return lhs.toLongLong() < rhs.toLongLong();
    case QVariant::ULongLong:
      return lhs.toULongLong() < rhs.toULongLong();
    case QVariant::Double:
      return lhs.toDouble() < rhs.toDouble();
    case QVariant::Char:
      return lhs.toChar() < rhs.toChar();
    case QVariant::Date:
      return lhs.toDate() < rhs.toDate();
    case QVariant::Time:
      return lhs.toTime() < rhs.toTime();
    case QVariant::DateTime:
      return lhs.toDateTime() < rhs.toDateTime();
    default:
      return QString::localeAwareCompare( lhs.toString(), rhs.toString() ) < 0;
  }
}

bool qgsVariantGreaterThan( const QVariant& lhs, const QVariant& rhs )
{
  return ! qgsVariantLessThan( lhs, rhs );
}

QString qgsInsertLinkAnchors( const QString& text )
{
  QString value = text;
  // http://alanstorm.com/url_regex_explained
  static QRegExp urlRegEx( "(\\b(([\\w-]+://?|www[.])[^\\s()<>]+(?:\\([\\w\\d]+\\)|([^!\"#$%&'()*+,\\-./:;<=>?@[\\\\\\]^_`{|}~\\s]|/))))" );
  static QRegExp protoRegEx( "^(?:f|ht)tps?://" );
  static QRegExp emailRegEx( "([\\w._%+-]+@[\\w.-]+\\.[A-Za-z]+)" );

  int offset = 0;
  while ( urlRegEx.indexIn( value, offset ) != -1 )
  {
    QString url = urlRegEx.cap( 1 );
    QString protoUrl = url;
    if ( protoRegEx.indexIn( protoUrl ) == -1 )
    {
      protoUrl.prepend( "http://" );
    }
    QString anchor = QString( "<a href=\"%1\">%2</a>" ).arg( Qt::escape( protoUrl ) ).arg( Qt::escape( url ) );
    value.replace( urlRegEx.pos( 1 ), url.length(), anchor );
    offset = urlRegEx.pos( 1 ) + anchor.length();
  }
  offset = 0;
  while ( emailRegEx.indexIn( value, offset ) != -1 )
  {
    QString email = emailRegEx.cap( 1 );
    QString anchor = QString( "<a href=\"mailto:%1\">%1</a>" ).arg( Qt::escape( email ) );
    value.replace( emailRegEx.pos( 1 ), email.length(), anchor );
    offset = emailRegEx.pos( 1 ) + anchor.length();
  }
  return value;
}

QString qgsVsiPrefix( QString path )
{
  if ( path.startsWith( "/vsizip/", Qt::CaseInsensitive ) ||
       path.endsWith( ".zip", Qt::CaseInsensitive ) )
    return "/vsizip/";
  else if ( path.startsWith( "/vsitar/", Qt::CaseInsensitive ) ||
            path.endsWith( ".tar", Qt::CaseInsensitive ) ||
            path.endsWith( ".tar.gz", Qt::CaseInsensitive ) ||
            path.endsWith( ".tgz", Qt::CaseInsensitive ) )
    return "/vsitar/";
  else if ( path.startsWith( "/vsigzip/", Qt::CaseInsensitive ) ||
            path.endsWith( ".gz", Qt::CaseInsensitive ) )
    return "/vsigzip/";
  else
    return "";
}

QGis::WkbType QGis::singleType( WkbType type )
{
  return ( QGis::WkbType )QgsWKBTypes::singleType(( QgsWKBTypes::Type )type );
}

QGis::WkbType QGis::multiType( WkbType type )
{
  return ( QGis::WkbType )QgsWKBTypes::multiType(( QgsWKBTypes::Type )type );
}

QGis::WkbType QGis::flatType( WkbType type )
{
  return ( QGis::WkbType )QgsWKBTypes::flatType(( QgsWKBTypes::Type )type );
}

bool QGis::isSingleType( WkbType type )
{
  return QgsWKBTypes::isSingleType(( QgsWKBTypes::Type )type );
}

bool QGis::isMultiType( WkbType type )
{
  return QgsWKBTypes::isMultiType(( QgsWKBTypes::Type )type );
}

int QGis::wkbDimensions( WkbType type )
{
  return QgsWKBTypes::wkbDimensions(( QgsWKBTypes::Type )type );
}
