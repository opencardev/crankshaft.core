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

#include "SettingsRegistry.h"

#include <algorithm>

namespace Crankshaft {

SettingsRegistry::SettingsRegistry(QObject *parent)
    : QObject(parent), m_currentComplexityLevel(Basic) {
  // Register default categories
  registerCategory("general", "General", "⚙", 10);
  registerCategory("appearance", "Appearance", "🎨", 20);
  registerCategory("connectivity", "Connectivity", "📡", 30);
  registerCategory("media", "Media", "🎵", 40);
  registerCategory("androidauto", "Android Auto", "🚗", 50);
  registerCategory("system", "System", "🖥", 90);
  registerCategory("about", "About", "ℹ", 100);
}

void SettingsRegistry::registerCategory(const QString &id, const QString &name, const QString &icon,
                                        int order) {
  SettingsCategory category;
  category.id = id;
  category.name = name;
  category.icon = icon;
  category.order = order;

  m_categories[id] = category;
  sortCategories();
  emit categoriesChanged();
}

void SettingsRegistry::registerPage(const QString &categoryId, const QString &pageId,
                                    const QString &name, const QString &icon,
                                    const QString &qmlComponent, int complexityLevel, int order) {
  if (!m_categories.contains(categoryId)) {
    qWarning() << "Category" << categoryId << "not found for page" << pageId;
    return;
  }

  SettingsPage page;
  page.id = pageId;
  page.name = name;
  page.icon = icon;
  page.qmlComponent = qmlComponent;
  page.categoryId = categoryId;
  page.complexityLevel = complexityLevelFromInt(complexityLevel);
  page.order = order;

  m_pages[pageId] = page;

  if (!m_categories[categoryId].pageIds.contains(pageId)) {
    m_categories[categoryId].pageIds.append(pageId);
    sortPagesInCategory(categoryId);
  }

  emit categoriesChanged();
}

void SettingsRegistry::registerSetting(const QString &pageId, const QVariantMap &setting) {
  if (!m_pages.contains(pageId)) {
    qWarning() << "Page" << pageId << "not found for setting" << setting.value("key").toString();
    return;
  }

  SettingDefinition def;
  def.key = setting.value("key").toString();
  def.label = setting.value("label").toString();
  def.description = setting.value("description").toString();
  def.type = setting.value("type").toString();
  def.defaultValue = setting.value("defaultValue");
  def.currentValue = setting.value("currentValue", def.defaultValue);
  def.options = setting.value("options").toMap();
  def.complexityLevel = complexityLevelFromInt(setting.value("complexityLevel", Basic).toInt());
  def.order = setting.value("order", 100).toInt();

  m_pages[pageId].settings.append(def);

  // Sort settings by order
  std::sort(
      m_pages[pageId].settings.begin(), m_pages[pageId].settings.end(),
      [](const SettingDefinition &a, const SettingDefinition &b) { return a.order < b.order; });
}

QVariantList SettingsRegistry::getPagesForCategory(const QString &categoryId) const {
  QVariantList result;

  if (!m_categories.contains(categoryId)) {
    return result;
  }

  for (const QString &pageId : m_categories[categoryId].pageIds) {
    if (m_pages.contains(pageId)) {
      const auto &page = m_pages[pageId];
      // Filter by complexity level
      if (page.complexityLevel <= m_currentComplexityLevel) {
        result.append(pageToVariantMap(page));
      }
    }
  }

  return result;
}

QVariantList SettingsRegistry::getSettingsForPage(const QString &pageId) const {
  QVariantList result;

  if (!m_pages.contains(pageId)) {
    return result;
  }

  for (const auto &setting : m_pages[pageId].settings) {
    // Filter by complexity level
    if (setting.complexityLevel <= m_currentComplexityLevel) {
      result.append(settingToVariantMap(setting));
    }
  }

  return result;
}

QVariantMap SettingsRegistry::getPage(const QString &pageId) const {
  if (!m_pages.contains(pageId)) {
    return QVariantMap();
  }

  return pageToVariantMap(m_pages[pageId]);
}

QString SettingsRegistry::getCategoryForPage(const QString &pageId) const {
  if (!m_pages.contains(pageId)) {
    return QString();
  }

  return m_pages[pageId].categoryId;
}

QVariantList SettingsRegistry::categories() const {
  QVariantList result;

  // Create sorted list
  QList<SettingsCategory> sortedCategories = m_categories.values();
  std::sort(sortedCategories.begin(), sortedCategories.end(),
            [](const SettingsCategory &a, const SettingsCategory &b) { return a.order < b.order; });

  for (const auto &category : sortedCategories) {
    // Only include categories that have visible pages
    bool hasVisiblePages = false;
    for (const QString &pageId : category.pageIds) {
      if (m_pages.contains(pageId) && m_pages[pageId].complexityLevel <= m_currentComplexityLevel) {
        hasVisiblePages = true;
        break;
      }
    }

    if (hasVisiblePages || category.pageIds.isEmpty()) {
      QVariantMap catMap;
      catMap["id"] = category.id;
      catMap["name"] = category.name;
      catMap["icon"] = category.icon;
      catMap["order"] = category.order;
      catMap["pageCount"] = getPagesForCategory(category.id).count();
      result.append(catMap);
    }
  }

  return result;
}

QString SettingsRegistry::currentComplexityLevel() const {
  return complexityLevelToString(m_currentComplexityLevel);
}

void SettingsRegistry::setCurrentComplexityLevel(const QString &level) {
  ComplexityLevel newLevel = complexityLevelFromString(level);
  if (newLevel != m_currentComplexityLevel) {
    m_currentComplexityLevel = newLevel;
    emit currentComplexityLevelChanged();
    emit categoriesChanged();
  }
}

void SettingsRegistry::sortCategories() {
  // Categories are sorted in the categories() getter
}

void SettingsRegistry::sortPagesInCategory(const QString &categoryId) {
  if (!m_categories.contains(categoryId)) {
    return;
  }

  auto &pageIds = m_categories[categoryId].pageIds;
  std::sort(pageIds.begin(), pageIds.end(), [this](const QString &a, const QString &b) {
    if (!m_pages.contains(a) || !m_pages.contains(b)) {
      return false;
    }
    return m_pages[a].order < m_pages[b].order;
  });
}

QVariantMap SettingsRegistry::pageToVariantMap(const SettingsPage &page) const {
  QVariantMap map;
  map["id"] = page.id;
  map["name"] = page.name;
  map["icon"] = page.icon;
  map["qmlComponent"] = page.qmlComponent;
  map["categoryId"] = page.categoryId;
  map["complexityLevel"] = static_cast<int>(page.complexityLevel);
  map["order"] = page.order;
  return map;
}

QVariantMap SettingsRegistry::settingToVariantMap(const SettingDefinition &setting) const {
  QVariantMap map;
  map["key"] = setting.key;
  map["label"] = setting.label;
  map["description"] = setting.description;
  map["type"] = setting.type;
  map["defaultValue"] = setting.defaultValue;
  map["currentValue"] = setting.currentValue;
  map["options"] = setting.options;
  map["complexityLevel"] = static_cast<int>(setting.complexityLevel);
  map["order"] = setting.order;
  return map;
}

SettingsRegistry::ComplexityLevel SettingsRegistry::complexityLevelFromInt(int level) const {
  switch (level) {
    case 0:
      return Basic;
    case 1:
      return Advanced;
    case 2:
      return Expert;
    case 3:
      return Developer;
    default:
      return Basic;
  }
}

QString SettingsRegistry::complexityLevelToString(ComplexityLevel level) const {
  switch (level) {
    case Basic:
      return "basic";
    case Advanced:
      return "advanced";
    case Expert:
      return "expert";
    case Developer:
      return "developer";
    default:
      return "basic";
  }
}

SettingsRegistry::ComplexityLevel SettingsRegistry::complexityLevelFromString(
    const QString &level) const {
  QString lower = level.toLower();
  if (lower == "advanced") return Advanced;
  if (lower == "expert") return Expert;
  if (lower == "developer") return Developer;
  return Basic;
}

}  // namespace Crankshaft
