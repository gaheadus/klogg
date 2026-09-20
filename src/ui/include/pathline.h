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

#ifndef KLOGG_PATHLINE_H
#define KLOGG_PATHLINE_H

#include "infoline.h"

class PathLine : public InfoLine
{
  Q_OBJECT

public:
    void setPath(const QString& path);

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent( QPaintEvent* paintEvent ) override;
    void resizeEvent( QResizeEvent* event ) override;
    QSize sizeHint() const override;

  private:
    void updateDisplayText();

    QString ellipsizedDisplay() const;

    QString path_;
    // Rendered (possibly ellipsized) form of path_. Kept in sync with
    // path_ in setPath()/resizeEvent() so the QLabel's internal text (used
    // by "Select all", "Copy" with a selection, accessibility, etc.) always
    // reflects the visible string while path_ remains the unsanitized
    // original.
    QString displayText_;
};


#endif // KLOGG_PATHLINE_H
