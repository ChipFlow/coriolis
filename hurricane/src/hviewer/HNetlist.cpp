
// -*- C++ -*-
//
// This file is part of the Coriolis Project.
// Copyright (C) Laboratoire LIP6 - Departement ASIM
// Universite Pierre et Marie Curie
//
// Main contributors :
//        Christophe Alexandre   <Christophe.Alexandre@lip6.fr>
//        Sophie Belloeil             <Sophie.Belloeil@lip6.fr>
//        Hugo Cl�ment                   <Hugo.Clement@lip6.fr>
//        Jean-Paul Chaput           <Jean-Paul.Chaput@lip6.fr>
//        Damien Dupuis                 <Damien.Dupuis@lip6.fr>
//        Christian Masson           <Christian.Masson@lip6.fr>
//        Marek Sroka                     <Marek.Sroka@lip6.fr>
// 
// The  Coriolis Project  is  free software;  you  can redistribute it
// and/or modify it under the  terms of the GNU General Public License
// as published by  the Free Software Foundation; either  version 2 of
// the License, or (at your option) any later version.
// 
// The  Coriolis Project is  distributed in  the hope that it  will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY  or FITNESS FOR  A PARTICULAR PURPOSE.   See the
// GNU General Public License for more details.
// 
// You should have  received a copy of the  GNU General Public License
// along with the Coriolis Project; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
// License-Tag
// Authors-Tag
// ===================================================================
//
// $Id$
//
// x-----------------------------------------------------------------x 
// |                                                                 |
// |                  H U R R I C A N E                              |
// |                                                                 |
// |  Author      :                    Jean-Paul CHAPUT              |
// |  E-mail      :       Jean-Paul.Chaput@asim.lip6.fr              |
// | =============================================================== |
// |  C++ Module  :       "./HNetlist.cpp"                           |
// | *************************************************************** |
// |  U p d a t e s                                                  |
// |                                                                 |
// x-----------------------------------------------------------------x


#include  <QFontMetrics>
#include  <QLabel>
#include  <QLineEdit>
#include  <QHeaderView>
#include  <QKeyEvent>
#include  <QGroupBox>
#include  <QVBoxLayout>

#include "hurricane/Commons.h"
#include "hurricane/Net.h"

#include "hurricane/viewer/Graphics.h"
#include "hurricane/viewer/HNetlistModel.h"
#include "hurricane/viewer/HNetlist.h"
#include "hurricane/viewer/HInspectorWidget.h"


namespace Hurricane {



  HNetlist::HNetlist ( QWidget* parent )
      : QWidget(parent)
      , _baseModel(NULL)
      , _sortModel(NULL)
      , _view(NULL)
      , _rowHeight(20)
      , _cellWidget(NULL)
  {
    setAttribute ( Qt::WA_DeleteOnClose );
    setAttribute ( Qt::WA_QuitOnClose, false );

    _rowHeight = QFontMetrics(Graphics::getFixedFont()).height() + 4;

    _baseModel = new HNetlistModel ( this );

    _sortModel = new QSortFilterProxyModel ( this );
    _sortModel->setSourceModel       ( _baseModel );
    _sortModel->setDynamicSortFilter ( true );
    _sortModel->setFilterKeyColumn   ( 0 );

    _view = new QTableView(this);
    _view->setShowGrid(false);
    _view->setAlternatingRowColors(true);
    _view->setSelectionBehavior(QAbstractItemView::SelectRows);
    _view->setSortingEnabled(true);
    _view->setModel ( _sortModel );
    _view->horizontalHeader()->setStretchLastSection ( true );

    QHeaderView* horizontalHeader = _view->horizontalHeader ();
    horizontalHeader->setStretchLastSection ( true );
    horizontalHeader->setMinimumSectionSize ( 200 );

    QHeaderView* verticalHeader = _view->verticalHeader ();
    verticalHeader->setVisible ( false );

    _filterPatternLineEdit = new QLineEdit(this);
    QLabel* filterPatternLabel = new QLabel(tr("&Filter pattern:"), this);
    filterPatternLabel->setBuddy(_filterPatternLineEdit);

    QGridLayout* inspectorLayout = new QGridLayout();
    inspectorLayout->addWidget(_view          , 1, 0, 1, 2);
    inspectorLayout->addWidget(filterPatternLabel    , 2, 0);
    inspectorLayout->addWidget(_filterPatternLineEdit, 2, 1);

    setLayout ( inspectorLayout );

    connect ( _filterPatternLineEdit, SIGNAL(textChanged(const QString &))
            , this                  , SLOT(textFilterChanged())
            );
    connect ( _view          , SIGNAL(activated(const QModelIndex&))
            , this                  , SLOT(selectNet(const QModelIndex&))
            );
    connect ( _baseModel , SIGNAL(layoutChanged()), this, SLOT(forceRowHeight()) );

    setWindowTitle(tr("Netlist"));
    resize(500, 300);

  }


  void  HNetlist::forceRowHeight ()
  {
    for (  int rows=_sortModel->rowCount()-1; rows >= 0 ; rows-- )
      _view->setRowHeight ( rows, _rowHeight );
  }


  void  HNetlist::selectNet ( const QModelIndex& index )
  {
    const Net* net = _baseModel->getNet ( _sortModel->mapToSource(index).row() );
    
    if ( _cellWidget && net ) {
      _cellWidget->unselectAll ();
      _cellWidget->select ( net );
    }
  }


  void  HNetlist::keyPressEvent ( QKeyEvent* event )
  {
    if ( event->key() == Qt::Key_I ) { runInspector(_view->currentIndex()); }
    else {
      event->ignore();
    }
  }


  void  HNetlist::textFilterChanged ()
  {
    _sortModel->setFilterRegExp ( _filterPatternLineEdit->text() );
    forceRowHeight ();
  }


  void  HNetlist::runInspector ( const QModelIndex& index  )
  {
    if ( index.isValid() ) {
      const Net* net = _baseModel->getNet ( _sortModel->mapToSource(index).row() );

      HInspectorWidget* inspector = new HInspectorWidget ();

      inspector->setRootRecord ( getRecord(net) );
      inspector->show ();
    }
  }


} // End of Hurricane namespace.
