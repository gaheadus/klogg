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
#include <QFileInfo>
#include <QFontMetrics>
#include <QMenu>
#include <QPainter>
#include <QResizeEvent>
#include <QString>

#include "containers.h"
#include "openfilehelper.h"
#include "clipboard.h"

void PathLine::setPath( const QString& path )
{
    path_ = path;
    // The user requirement is explicit: path_ must never be modified, and
    // the file name/path stored in klogg must remain intact. We only
    // compute a separate ellipsized string used for display. setText() is
    // called here (NOT inside paintEvent) so the QLabel's internal text
    // matches what is visually rendered — this keeps "Select all" and
    // text-selection-based "Copy" functioning as users expect, while
    // path_ stays untouched.
    updateDisplayText();
}

void PathLine::resizeEvent( QResizeEvent* event )
{
    QLabel::resizeEvent( event );
    // Width changed → recompute the ellipsized display string so it fits.
    updateDisplayText();
}

void PathLine::updateDisplayText()
{
    const QString next = ellipsizedDisplay();
    if ( next == displayText_ ) {
        return;
    }
    displayText_ = next;
    setText( displayText_ );
}

QSize PathLine::sizeHint() const
{
    // Don't use path_'s full width — doing so would let a long filename force
    // the toolbar to grow. Qt will happily clip our paint output instead.
    return QLabel::sizeHint();
}

QString PathLine::ellipsizedDisplay() const
{
    if ( path_.isEmpty() ) {
        return QString();
    }

    const QFontMetrics fm = fontMetrics();
    const int availableWidth = width();
    if ( availableWidth <= 0 ) {
        return path_;
    }

    const int fullWidth = fm.horizontalAdvance( path_ );
    if ( fullWidth <= availableWidth ) {
        return path_;
    }

    // Need to ellipsize. Per requirement, when path+filename is too long,
    // prioritize showing the FILENAME on the right and drop characters from
    // the LEFT side of the path prefix. We never modify path_ itself.
    const QString fileName = QFileInfo( path_ ).fileName();
    const QString ellipsis = QStringLiteral( "..." );
    const int ellipsisWidth = fm.horizontalAdvance( ellipsis );

    // Helper: progressively remove characters from the beginning of `base`
    // (keeping the full suffix) until the resulting text fits `targetWidth`.
    auto fitSuffixWidth = [&]( const QString& base, int targetWidth ) -> QString {
        if ( fm.horizontalAdvance( base ) <= targetWidth ) {
            return base;
        }
        // Strip one character from the left at a time; this preserves the
        // rightmost (filename / path tail) portion intact.
        int drop = 0;
        while ( drop < base.size()
                && fm.horizontalAdvance( ellipsis + base.mid( drop + 1 ) )
                       > targetWidth ) {
            ++drop;
        }
        return ellipsis + base.mid( drop + 1 );
    };

    if ( !fileName.isEmpty() && fileName != path_ ) {
        const int fileNameWidth = fm.horizontalAdvance( fileName );
        if ( fileNameWidth <= availableWidth ) {
            // Filename fits; show "..." + as much of the left path as fits.
            const int pathBudget = availableWidth - fileNameWidth;
            if ( pathBudget >= ellipsisWidth ) {
                const QString left = path_.left( path_.size() - fileName.size() );
                const QString shortLeft = fitSuffixWidth( left, pathBudget );
                return shortLeft + fileName;
            }
            // No room for prefix at all; just show filename.
            return fileName;
        }
        // Filename itself overflows; truncate from the left of the filename.
        return fitSuffixWidth( fileName, availableWidth );
    }

    // path_ has no separate filename portion (e.g. trailing slash) — fall
    // back to left-truncating the whole path.
    return fitSuffixWidth( path_, availableWidth );
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

    connect( selectAll, &QAction::triggered, this, [ this ]( auto ) {
        // Select all operates on the rendered ellipsized display text.
        setSelection( 0, klogg::isize( this->ellipsizedDisplay() ) );
    } );

    menu.exec( event->globalPos() );
}

void PathLine::paintEvent( QPaintEvent* paintEvent )
{
    // QLabel's text is set in setPath() to the ellipsized display string.
    // We just need to honor the parent's background fill (InfoLine adds a
    // gradient gauge during searches) and let QLabel draw its own text.
    InfoLine::paintEvent( paintEvent );
}

