/*
 * Copyright (C) 2009, 2010, 2012, 2017 Nicolas Bonnefon and other contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

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

// This file implements the FilteredView concrete class.
// Most of the actual drawing and event management is done in AbstractLogView
// Only behaviour specific to the filtered (bottom) view is implemented here.

#include <cassert>

#include "filteredview.h"
#include "shortcuts.h"

FilteredView::FilteredView( std::shared_ptr<LogFilteredData> newLogData,
                            const QuickFindPattern* const quickFindPattern, QWidget* parent )
    : AbstractLogView( newLogData.get(), quickFindPattern, parent )
{
    // FilteredView owns its LogFilteredData via shared_ptr. This guarantees
    // the data outlives the view (or, conversely, is destroyed together
    // with the view if no other owner keeps a reference), eliminating the
    // dangling-pointer window where the view still references a LogFilteredData
    // that has already been freed from the CrawlerWidget's map.
    logFilteredData_ = std::move( newLogData );
}

FilteredView::~FilteredView()
{
    // Stop quick find search before AbstractLogView (and its quickFind_) are
    // destroyed. This is also done in AbstractLogView::~AbstractLogView but
    // doing it here ensures the worker is stopped while this view is still
    // fully alive (its vtable is still FilteredView's), so any signal handlers
    // queued by the worker can safely reach us.
    stopQuickFindSearch();

    // Drop our reference to the LogFilteredData NOW. This makes the destruction
    // order explicit: we destroy the data after stopping QuickFind (which still
    // references it) and before AbstractLogView::logData_ becomes invalid.
    // If any external code still holds a shared_ptr, the data lives on; if
    // not, it is destroyed here in a well-defined order.
    logFilteredData_.reset();
}

void FilteredView::stopSearch()
{
    stopQuickFindSearch();
}

void FilteredView::setVisibility( Visibility visi )
{
    assert( logFilteredData_ );

    logFilteredData_->setVisibility( visi );

    updateData();
}

FilteredView::Visibility FilteredView::visibility() const
{
    assert( logFilteredData_ );

    return logFilteredData_->visibility();
}

// For the filtered view, a line is always matching!
AbstractLogData::LineType FilteredView::lineType( LineNumber lineNumber ) const
{
    // line in filteredview corresponds to index
    if ( !logFilteredData_ ) {
        return AbstractLogData::LineTypeFlags::Plain;
    }
    return logFilteredData_->lineTypeByIndex( lineNumber );
}

LineNumber FilteredView::displayLineNumber( LineNumber lineNumber ) const
{
    if ( !logFilteredData_ ) {
        return 1_lnum;
    }
    // Display a 1-based index
    return logFilteredData_->getMatchingLineNumber( lineNumber ) + 1_lcount;
}

LineNumber FilteredView::lineIndex( LineNumber lineNumber ) const
{
    if ( !logFilteredData_ ) {
        return lineNumber;
    }
    return logFilteredData_->getLineIndexNumber( lineNumber );
}

LineNumber FilteredView::maxDisplayLineNumber() const
{
    if ( !logFilteredData_ ) {
        return 1_lnum;
    }
    return LineNumber( logFilteredData_->getNbTotalLines().get() );
}

void FilteredView::doRegisterShortcuts()
{
    LOG_INFO << "Registering shortcuts for filtered view";
    AbstractLogView::doRegisterShortcuts();
    registerShortcut( ShortcutAction::LogViewNextMark, [ this ] {
        using LineTypeFlags = LogFilteredData::LineTypeFlags;
        if ( !logFilteredData_ ) {
            return;
        }
        auto i = getViewPosition() - 1_lcount;
        bool foundMark = false;
        for ( ; i != 0_lnum; --i ) {
            if ( lineType( i ).testFlag( LineTypeFlags::Mark ) ) {
                foundMark = true;
                break;
            }
        }

        if ( !foundMark ) {
            foundMark = lineType( i ).testFlag( LineTypeFlags::Mark );
        }

        if ( foundMark ) {
            selectAndDisplayLine( i );
        }
    } );
    registerShortcut( ShortcutAction::LogViewPrevMark, [ this ] {
        if ( !logFilteredData_ ) {
            return;
        }
        const auto nbLines = logFilteredData_->getNbLine();
        for ( auto i = getViewPosition() + 1_lcount; i < nbLines; ++i ) {
            if ( lineType( i ).testFlag( LogFilteredData::LineTypeFlags::Mark ) ) {
                selectAndDisplayLine( i );
                break;
            }
        }
    } );
}
