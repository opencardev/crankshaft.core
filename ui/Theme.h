/*
 * Project: Crankshaft
 * This file is part of Crankshaft project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  Crankshaft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crankshaft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef THEME_H
#define THEME_H

#include <QObject>
#include <QColor>
#include <qqml.h>

class Theme : public QObject {
    Q_OBJECT

    // Core colors
    Q_PROPERTY(bool isDark READ isDark WRITE setIsDark NOTIFY isDarkChanged)
    Q_PROPERTY(QColor background READ background NOTIFY themeChanged)
    Q_PROPERTY(QColor surface READ surface NOTIFY themeChanged)
    Q_PROPERTY(QColor surfaceVariant READ surfaceVariant NOTIFY themeChanged)
    Q_PROPERTY(QColor primary READ primary NOTIFY themeChanged)
    Q_PROPERTY(QColor primaryVariant READ primaryVariant NOTIFY themeChanged)
    Q_PROPERTY(QColor secondary READ secondary NOTIFY themeChanged)
    Q_PROPERTY(QColor error READ error NOTIFY themeChanged)
    Q_PROPERTY(QColor success READ success NOTIFY themeChanged)
    Q_PROPERTY(QColor warning READ warning NOTIFY themeChanged)
    
    // Text colors
    Q_PROPERTY(QColor textPrimary READ textPrimary NOTIFY themeChanged)
    Q_PROPERTY(QColor textSecondary READ textSecondary NOTIFY themeChanged)
    Q_PROPERTY(QColor textDisabled READ textDisabled NOTIFY themeChanged)
    Q_PROPERTY(QColor divider READ divider NOTIFY themeChanged)
    
    // Font sizes
    Q_PROPERTY(int fontSizeDisplay READ fontSizeDisplay CONSTANT)
    Q_PROPERTY(int fontSizeHeading1 READ fontSizeHeading1 CONSTANT)
    Q_PROPERTY(int fontSizeHeading2 READ fontSizeHeading2 CONSTANT)
    Q_PROPERTY(int fontSizeHeading3 READ fontSizeHeading3 CONSTANT)
    Q_PROPERTY(int fontSizeBody READ fontSizeBody CONSTANT)
    Q_PROPERTY(int fontSizeCaption READ fontSizeCaption CONSTANT)
    
    // Spacing 
    Q_PROPERTY(int spacingXs READ spacingXs CONSTANT)
    Q_PROPERTY(int spacingSm READ spacingSm CONSTANT)
    Q_PROPERTY(int spacingMd READ spacingMd CONSTANT)
    Q_PROPERTY(int spacingLg READ spacingLg CONSTANT)
    Q_PROPERTY(int spacingXl READ spacingXl CONSTANT)
    
    // Border radius
    Q_PROPERTY(int radiusSm READ radiusSm CONSTANT)
    Q_PROPERTY(int radiusMd READ radiusMd CONSTANT)
    Q_PROPERTY(int radiusLg READ radiusLg CONSTANT)
    Q_PROPERTY(int radiusXl READ radiusXl CONSTANT)
    
    // Elevation/Shadow
    Q_PROPERTY(int elevationLow READ elevationLow CONSTANT)
    Q_PROPERTY(int elevationMedium READ elevationMedium CONSTANT)
    Q_PROPERTY(int elevationHigh READ elevationHigh CONSTANT)
    
    // Animation
    Q_PROPERTY(int animationDuration READ animationDuration CONSTANT)

public:
    explicit Theme(QObject *parent = nullptr) : QObject(parent), m_isDark(true) {}

    // Core colors
    bool isDark() const { return m_isDark; }
    void setIsDark(bool dark) {
        if (m_isDark != dark) {
            m_isDark = dark;
            emit isDarkChanged();
            emit themeChanged();
        }
    }

    QColor background() const { return m_isDark ? QColor("#0D1117") : QColor("#FFFFFF"); }
    QColor surface() const { return m_isDark ? QColor("#161B22") : QColor("#F6F8FA"); }
    QColor surfaceVariant() const { return m_isDark ? QColor("#1C2128") : QColor("#EAEEF2"); }
    QColor primary() const { return QColor("#2188FF"); }
    QColor primaryVariant() const { return QColor("#0366D6"); }
    QColor secondary() const { return QColor("#6F42C1"); }
    QColor error() const { return QColor("#F85149"); }
    QColor success() const { return QColor("#56D364"); }
    QColor warning() const { return QColor("#D29922"); }
    
    // Text colors
    QColor textPrimary() const { return m_isDark ? QColor("#E6EDF3") : QColor("#24292F"); }
    QColor textSecondary() const { return m_isDark ? QColor("#8B949E") : QColor("#57606A"); }
    QColor textDisabled() const { return m_isDark ? QColor("#484F58") : QColor("#8C959F"); }
    QColor divider() const { return m_isDark ? QColor("#21262D") : QColor("#D0D7DE"); }
    
    // Font sizes
    int fontSizeDisplay() const { return 32; }
    int fontSizeHeading1() const { return 24; }
    int fontSizeHeading2() const { return 18; }
    int fontSizeHeading3() const { return 16; }
    int fontSizeBody() const { return 14; }
    int fontSizeCaption() const { return 12; }
    
    // Spacing
    int spacingXs() const { return 4; }
    int spacingSm() const { return 8; }
    int spacingMd() const { return 16; }
    int spacingLg() const { return 24; }
    int spacingXl() const { return 32; }
    
    // Border radius
    int radiusSm() const { return 4; }
    int radiusMd() const { return 8; }
    int radiusLg() const { return 12; }
    int radiusXl() const { return 16; }
    
    // Elevation/Shadow
    int elevationLow() const { return 2; }
    int elevationMedium() const { return 4; }
    int elevationHigh() const { return 8; }
    
    // Animation
    int animationDuration() const { return 200; }

    Q_INVOKABLE void toggleTheme() {
        setIsDark(!m_isDark);
    }

signals:
    void isDarkChanged();
    void themeChanged();

private:
    bool m_isDark;
};

#endif // THEME_H
