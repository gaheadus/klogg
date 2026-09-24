/*
 * Copyright (C) 2009, 2010, 2012 Nicolas Bonnefon and other contributors
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

#ifndef FILTEREDVIEW_H
#define FILTEREDVIEW_H

#include "abstractlogview.h"

#include "logfiltereddata.h"

#include <QKeyEvent>

#include <memory>

// Class implementing the filtered (bottom) view widget.
class FilteredView : public AbstractLogView
{
  Q_OBJECT
  public:
    FilteredView( std::shared_ptr<LogFilteredData> newLogData,
            const QuickFindPattern* const quickFindPattern,
            QWidget* parent = nullptr );
    ~FilteredView() override;

    // Stop any ongoing quick find search. Call this before destroying the view
    // to prevent crashes from accessing the view while it's being destroyed.
    void stopSearch();

    // Returns the LogFilteredData this view is bound to.
    // Returns nullptr if the data has been reset (view destroyed).
    std::shared_ptr<LogFilteredData> filteredData() const { return logFilteredData_; }

    // What is visible in the view.
    using Visibility = LogFilteredData::Visibility;
    void setVisibility( Visibility visi );
    Visibility visibility() const;

  protected:
    LogFilteredData::LineType lineType( LineNumber lineNumber ) const override;

    // Number of the filtered line relative to the unfiltered source
    LineNumber displayLineNumber(LineNumber lineNumber ) const override;
    LineNumber lineIndex( LineNumber lineNumber ) const override;
    LineNumber maxDisplayLineNumber() const override;

    void doRegisterShortcuts() override;

  private:
    // LogFilteredData is owned by FilteredView via shared_ptr. This guarantees
    // the data lives as long as the view does; no other code can drop the
    // last reference and destroy the data while the view is still alive.
    std::shared_ptr<LogFilteredData> logFilteredData_;
};

#endif
