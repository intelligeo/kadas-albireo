/***************************************************************************
 *  qgscatalogbrowser.cpp                                                  *
 *  ---------------------                                                  *
 *  begin                : January, 2016                                   *
 *  copyright            : (C) 2016 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgscatalogbrowser.h"
#include "qgscatalogprovider.h"
#include "qgsarcgisrestcatalogprovider.h"
#include "qgsgeoadminrestcatalogprovider.h"
#include "qgsvbscatalogprovider.h"
#include "qgsfilterlineedit.h"
#include "qgsmimedatautils.h"
#include "qgsrasterlayer.h"
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QTreeView>
#include <QVBoxLayout>
#include <QTextStream>
#include <QUrl>


class QgsCatalogBrowser::CatalogItem : public QStandardItem
{
  public:
    CatalogItem() : QStandardItem() {}
    CatalogItem( const QString &text ) : QStandardItem( text ) {}
    CatalogItem( const QIcon &icon, const QString &text ) : QStandardItem( icon, text ) {}
    explicit CatalogItem( int rows, int columns = 1 ) : QStandardItem( rows, columns ) {}

    bool operator<( const QStandardItem &other ) const
    {
      int i1 = data( s_sortIndexRole ).toInt();
      int i2 = other.data( s_sortIndexRole ).toInt();
      return i1 >= 0 && i2 >= 0 ? i1 < i2 : QStandardItem::operator <( other );
    }

    static const int s_uriRole;
    static const int s_sortIndexRole;
};


const int QgsCatalogBrowser::CatalogItem::s_uriRole = Qt::UserRole + 1;
const int QgsCatalogBrowser::CatalogItem::s_sortIndexRole = Qt::UserRole + 2;


class QgsCatalogBrowser::CatalogModel : public QStandardItemModel
{
  public:
    CatalogModel( QObject* parent = 0 ) : QStandardItemModel( parent )
    {
    }

    QStandardItem* addItem( QStandardItem* parent, const QString& value, int sortIndex, bool isLeaf, QMimeData* mimeData )
    {
      if ( !parent )
      {
        parent = invisibleRootItem();
      }
      // Create category group item if necessary
      if ( !isLeaf )
      {
        QStandardItem* groupItem = 0;
        for ( int i = 0, n = parent->rowCount(); i < n; ++i )
        {
          if ( parent->child( i )->text() == value )
          {
            groupItem = parent->child( i );
            break;
          }
        }
        if ( !groupItem )
        {
          groupItem = new CatalogItem( value );
          groupItem->setData( sortIndex, CatalogItem::s_sortIndexRole );
          groupItem->setDragEnabled( false );
          groupItem->setToolTip( value );
          parent->setChild( parent->rowCount(), groupItem );
        }
        return groupItem;
      }
      else
      {
        QStandardItem* item = new CatalogItem( value );
        parent->setChild( parent->rowCount(), item );
        item->setData( sortIndex, CatalogItem::s_sortIndexRole );
        item->setToolTip( value );
        item->setData( QgsMimeDataUtils::decodeUriList( mimeData ).front().data(), CatalogItem::s_uriRole );
        return item;
      }
    }

    QMimeData* mimeData( const QModelIndexList &indexes ) const override
    {
      QList<QModelIndex> indexStack;
      QModelIndex index = indexes.front();
      indexStack.prepend( index );
      while ( index.parent().isValid() )
      {
        index = index.parent();
        indexStack.prepend( index );
      }

      QStandardItem* item = itemFromIndex( indexStack.front() );
      if ( item )
      {
        for ( int i = 1, n = indexStack.size(); i < n; ++i )
        {
          item = item->child( indexStack[i].row() );
        }
        QgsMimeDataUtils::Uri uri( item->data( CatalogItem::s_uriRole ).toString() );

        QMimeData* data = QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << uri );
        return data;
      }
      return 0;
    }

    QStringList mimeTypes() const override
    {
      return QStringList() << "text/uri-list" << "application/x-vnd.qgis.qgis.uri";
    }
};

class QgsCatalogBrowser::TreeFilterProxyModel : public QSortFilterProxyModel
{
  public:
    TreeFilterProxyModel( QObject* parent = 0 ) : QSortFilterProxyModel( parent )
    {
    }

    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override
    {
      if ( QSortFilterProxyModel::filterAcceptsRow( source_row, source_parent ) )
      {
        return true;
      }
      QModelIndex parent = source_parent;
      while ( parent.isValid() )
      {
        if ( QSortFilterProxyModel::filterAcceptsRow( parent.row(), parent.parent() ) )
        {
          return true;
        }
        parent = parent.parent();
      }
      return acceptsAnyChildren( source_row, source_parent );
    }

  private:
    bool acceptsAnyChildren( int source_row, const QModelIndex &source_parent ) const
    {
      QModelIndex item = sourceModel()->index( source_row, 0, source_parent );
      if ( item.isValid() )
      {
        for ( int i = 0, n = item.model()->rowCount( item ); i < n; ++i )
        {
          if ( QSortFilterProxyModel::filterAcceptsRow( i, item ) || acceptsAnyChildren( i, item ) )
            return true;
        }
      }
      return false;
    }
};

QgsCatalogBrowser::QgsCatalogBrowser( QWidget *parent )
    : QWidget( parent )
{
  setLayout( new QVBoxLayout() );
  layout()->setContentsMargins( 0, 0, 0, 0 );
  layout()->setSpacing( 0 );

  mFilterLineEdit = new QgsFilterLineEdit( this );
  mFilterLineEdit->setPlaceholderText( tr( "Filter catalog..." ) );
  mFilterLineEdit->setObjectName( "mCatalogBrowserLineEdit" );
  mFilterLineEdit->setStyleSheet( "QLineEdit { border-style: solid; border-color: #e4e4e4; border-width: 1px 0px 1px 0px; }" );
  layout()->addWidget( mFilterLineEdit );
  connect( mFilterLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( filterChanged( QString ) ) );

  mTreeView = new QTreeView( this );
  mTreeView->setFrameShape( QTreeView::NoFrame );
  mTreeView->setEditTriggers( QTreeView::NoEditTriggers );
  mTreeView->setDragEnabled( true );
  mTreeView->setIndentation( 10 );
  layout()->addWidget( mTreeView );
  connect( mTreeView, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( itemDoubleClicked( QModelIndex ) ) );
  mCatalogModel = new CatalogModel( this );
  mTreeView->setHeaderHidden( true );

  mFilterProxyModel = new TreeFilterProxyModel( this );

  mLoadingModel = new QStandardItemModel( this );
  QStandardItem* loadingItem = new QStandardItem( tr( "Loading..." ) );
  loadingItem->setEnabled( false );
  mLoadingModel->appendRow( loadingItem );

  mOfflineModel = new QStandardItemModel( this );
  QStandardItem* offlineItem = new QStandardItem( tr( "Offline" ) );
  offlineItem->setEnabled( false );
  mOfflineModel->appendRow( offlineItem );

  QStringList catalogUris = QSettings().value( "/qgis/geodatacatalogs" ).toString().split( ";;" );
  foreach ( const QString& catalogUri, catalogUris )
  {
    QUrl u = QUrl::fromEncoded( "?" + catalogUri.toLocal8Bit() );
    QString type = u.queryItemValue( "type" );
    QString url = u.queryItemValue( "url" );
    if ( type == "geoadmin" )
    {
      addProvider( new QgsGeoAdminRestCatalogProvider( url, this ) );
    }
    else if ( type == "arcgisrest" )
    {
      addProvider( new QgsArcGisRestCatalogProvider( url, this ) );
    }
    else if ( type == "vbs" )
    {
      addProvider( new QgsVBSCatalogProvider( url, this ) );
    }
  }
}

void QgsCatalogBrowser::reload()
{
  if ( mTreeView->model() != mLoadingModel && mProviders.size() > 0 )
  {
    mCatalogModel->setRowCount( 0 );
    mTreeView->setModel( mLoadingModel );

    mFinishedProviders = 0;
    foreach ( QgsCatalogProvider* provider, mProviders )
    {
      connect( provider, SIGNAL( finished() ), this, SLOT( providerFinished() ) );
      provider->load();
    }
  }
}

void QgsCatalogBrowser::providerFinished()
{
  mFinishedProviders += 1;
  if ( mFinishedProviders == mProviders.size() )
  {
    if ( mCatalogModel->rowCount() > 0 )
    {
      mCatalogModel->invisibleRootItem()->sortChildren( 0 );
      mFilterProxyModel->setSourceModel( mCatalogModel );
      mTreeView->setModel( mFilterProxyModel );
    }
    else
    {
      mTreeView->setModel( mOfflineModel );
    }
  }
}

void QgsCatalogBrowser::filterChanged( const QString &text )
{
  mTreeView->clearSelection();
  mFilterProxyModel->setFilterFixedString( text );
  mFilterProxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
  if ( text.length() >= 3 )
  {
    mTreeView->expandAll();
  }
}

void QgsCatalogBrowser::itemDoubleClicked( const QModelIndex &index )
{
  QMimeData* data = mCatalogModel->mimeData( QModelIndexList() << mFilterProxyModel->mapToSource( index ) );
  if ( data )
  {
    QgsMimeDataUtils::UriList uriList = QgsMimeDataUtils::decodeUriList( data );
    if ( !uriList.isEmpty() && !uriList[0].uri.isEmpty() )
    {
      QString uri = QgisApp::instance()->crsAndFormatAdjustedLayerUri( uriList[0].uri, uriList[0].supportedCrs, uriList[0].supportedFormats );
      QgsRasterLayer* layer = QgisApp::instance()->addRasterLayer( uri, uriList[0].name, uriList[0].providerKey );
      if ( layer )
        layer->setInfoUrl( uriList[0].layerInfoUrl );
    }
    delete data;
  }
}

QStandardItem *QgsCatalogBrowser::addItem( QStandardItem *parent, QString text, int sortIndex, bool isLeaf, QMimeData *mimeData )
{
  return mCatalogModel->addItem( parent, text, sortIndex, isLeaf, mimeData );
}
