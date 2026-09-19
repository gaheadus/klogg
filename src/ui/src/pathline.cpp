/*
 * Copyright (C) 2016 -- 2019 Anton Filimonov and other contributors
 *
 * This file is part of klogg.
 *
 * klogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * klogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with klogg.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pathline.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>

#include "containers.h"
#include "openfilehelper.h"
#include "clipboard.h"

void PathLine::setPath( const QString& path )
{
    path_ = path;
    update();
}

QSize PathLine::sizeHint() const
{
    if ( path_.isEmpty() ) {
        return QLabel::sizeHint();
    }
    return QSize( fontMetrics().horizontalAdvance( path_ ), QLabel::sizeHint().height() );
}

void PathLine::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu menu( this );

    auto copyFullPath = menu.addAction( tr( "Copy full path" ) );
    auto copyFileName = menu.addAction( tr( "Copy file name" ) );
    auto openContainingFolder = menu.addAction( tr( "Open containing folder" ) );
    menu.addSeparator();
    auto copySelection = menu.addAction( tr( "Copy" ) );
    menu.addSeparator();
    auto selectAll = menu.addAction( tr( "Select all" ) );

    connect( copyFullPath, &QAction::triggered, this,
             [ this ]( auto ) { sendTextToClipboard( this->path_ ); } );

    connect( copyFileName, &QAction::triggered, this, [ this ]( auto ) {
        sendTextToClipboard( QFileInfo( this->path_ ).fileName() );
    } );

    connect( openContainingFolder, &QAction::triggered, this,
             [ this ]( auto ) { showPathInFileExplorer( this->path_ ); } );

    copySelection->setEnabled( this->hasSelectedText() );
    connect( copySelection, &QAction::triggered, this,
             [ this ]( auto ) { sendTextToClipboard( this->selectedText() ); } );

    connect( selectAll, &QAction::triggered, this,
             [ this ]( auto ) { setSelection( 0, klogg::isize( this->text() ) ); } );

    menu.exec( event->globalPos() );
}

void PathLine::paintEvent( QPaintEvent* paintEvent )
{
    if ( path_.isEmpty() ) {
        InfoLine::paintEvent( paintEvent );
        return;
    }

    const QFontMetrics fm = fontMetrics();
    const int fullWidth = fm.horizontalAdvance( path_ );
    const int availableWidth = width();

    QString textToDisplay;
    if ( fullWidth <= availableWidth ) {
        textToDisplay = path_;
    } else {
        const QString fileName = QFileInfo( path_ ).fileName();
        const int fileNameWidth = fm.horizontalAdvance( fileName );
        const int ellipsisWidth = fm.horizontalAdvance( QStringLiteral( "..." ) );

        if ( fileNameWidth + ellipsisWidth < availableWidth ) {
            const int pathWidth = availableWidth - fileNameWidth - ellipsisWidth;
            int pos = 0;
            int pathPixelWidth = 0;
            while ( pos < path_.size() && pathPixelWidth < pathWidth ) {
                pathPixelWidth += fm.horizontalAdvance( path_[ pos++ ] );
            }
            if ( pos >= path_.size() ) {
                textToDisplay = path_;
            } else {
                textToDisplay = QStringLiteral( "..." ) + path_.mid( pos );
            }
        } else {
            if ( fileNameWidth < availableWidth ) {
                textToDisplay = fileName;
            } else {
                const int ellipsisW = fm.horizontalAdvance( QStringLiteral( "..." ) );
                const int nameWidth = availableWidth - ellipsisW;
                int pos = fileName.size();
                int namePixelWidth = 0;
                while ( pos > 0 && namePixelWidth < nameWidth ) {
                    const QChar ch = fileName[ pos - 1 ];
                    namePixelWidth += fm.horizontalAdvance( ch );
                    --pos;
                }
                textToDisplay = QStringLiteral( "..." ) + fileName.mid( pos );
            }
        }
    }

    setText( textToDisplay );
    InfoLine::paintEvent( paintEvent );
}
