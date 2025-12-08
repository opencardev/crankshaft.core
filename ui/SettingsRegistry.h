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

#pragma once

#include <QObject>
#include <QVariantMap>
#include <QVariantList>
#include <QQmlEngine>

namespace Crankshaft {

class SettingsRegistry : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
    Q_PROPERTY(QVariantList categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(QString currentComplexityLevel READ currentComplexityLevel WRITE setCurrentComplexityLevel NOTIFY currentComplexityLevelChanged)

public:
    enum ComplexityLevel {
        Basic = 0,      // Essential settings only
        Advanced = 1,   // Common advanced settings
        Expert = 2,     // Power user settings
        Developer = 3   // Debug and development settings
    };
    Q_ENUM(ComplexityLevel)

    explicit SettingsRegistry(QObject *parent = nullptr);
    
    // Register a settings category
    Q_INVOKABLE void registerCategory(const QString &id, 
                                     const QString &name, 
                                     const QString &icon,
                                     int order = 100);
    
    // Register a settings page within a category
    Q_INVOKABLE void registerPage(const QString &categoryId,
                                  const QString &pageId,
                                  const QString &name,
                                  const QString &icon,
                                  const QString &qmlComponent,
                                  int complexityLevel = Basic,
                                  int order = 100);
    
    // Register individual settings
    Q_INVOKABLE void registerSetting(const QString &pageId,
                                    const QVariantMap &setting);
    
    // Get pages for a category filtered by complexity level
    Q_INVOKABLE QVariantList getPagesForCategory(const QString &categoryId) const;
    
    // Get settings for a page filtered by complexity level
    Q_INVOKABLE QVariantList getSettingsForPage(const QString &pageId) const;
    
    // Get page by ID (for deep linking)
    Q_INVOKABLE QVariantMap getPage(const QString &pageId) const;
    
    // Get category containing page
    Q_INVOKABLE QString getCategoryForPage(const QString &pageId) const;
    
    QVariantList categories() const;
    QString currentComplexityLevel() const;
    void setCurrentComplexityLevel(const QString &level);

signals:
    void categoriesChanged();
    void currentComplexityLevelChanged();
    void settingChanged(const QString &key, const QVariant &value);

private:
    struct SettingDefinition {
        QString key;
        QString label;
        QString description;
        QString type;  // "toggle", "select", "text", "number", "slider"
        QVariant defaultValue;
        QVariant currentValue;
        QVariantMap options;  // For selects, sliders, etc.
        ComplexityLevel complexityLevel;
        int order;
    };
    
    struct SettingsPage {
        QString id;
        QString name;
        QString icon;
        QString qmlComponent;
        QString categoryId;
        ComplexityLevel complexityLevel;
        int order;
        QList<SettingDefinition> settings;
    };
    
    struct SettingsCategory {
        QString id;
        QString name;
        QString icon;
        int order;
        QList<QString> pageIds;
    };
    
    QMap<QString, SettingsCategory> m_categories;
    QMap<QString, SettingsPage> m_pages;
    ComplexityLevel m_currentComplexityLevel;
    
    void sortCategories();
    void sortPagesInCategory(const QString &categoryId);
    QVariantMap pageToVariantMap(const SettingsPage &page) const;
    QVariantMap settingToVariantMap(const SettingDefinition &setting) const;
    ComplexityLevel complexityLevelFromInt(int level) const;
    QString complexityLevelToString(ComplexityLevel level) const;
    ComplexityLevel complexityLevelFromString(const QString &level) const;
};

} // namespace Crankshaft
