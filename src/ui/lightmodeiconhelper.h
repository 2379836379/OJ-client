#pragma once

#include <QString>

class QPushButton;
class QWidget;

namespace LightModeIconHelper
{
void setDarkModeEnabled(bool enabled);
void applyIcon(QPushButton *button,
               const QString &fileName,
               bool iconOnly = true,
               int iconSize = 20);
void refreshIcon(QPushButton *button);
void refreshIcons(QWidget *root);
void applyToolsToggleIcon(QPushButton *button, bool expanded);
}
