#pragma once

#include <QFile>
#include <QString>

namespace UiTheme
{
struct Palette
{
    QString page;
    QString border;
    QString text;
    QString selected;
    QString hover;
};
//页面外层和内容区之间的间距；首页双栏之间也使用它。
inline constexpr int PageSpacing = 4;
//侧边工具栏展开和折叠时的宽度
inline constexpr int ToolRailExpandedWidth = 240;
inline constexpr int ToolRailCollapsedWidth = 84;
//圆角半径
inline constexpr int PanelRadius = 4;

inline Palette palette(bool dark)
{
    if (dark) {
        return {"#1E1E1E",
                "#303031",
                "#D4D4D4",
                "#007ACC",
                "#2A2D2E"};
    }

    return {"#FFFFFF",
            "#E5E5E5",
            "#000000",
            "#CCE5FF",
            "#E8E8E8"};
}

inline QString pageShellStyleSheet(bool dark)
{
    const Palette colors = palette(dark);
    QFile styleFile(QStringLiteral(":/src/ui/styles/pageshell.qss"));
    if (!styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qFatal("Unable to load the PageShell stylesheet resource");
    }

    QString style = QString::fromUtf8(styleFile.readAll());
    style.replace(QStringLiteral("@page@"), colors.page);
    style.replace(QStringLiteral("@border@"), colors.border);
    style.replace(QStringLiteral("@radius@"), QString::number(PanelRadius));
    style.replace(QStringLiteral("@text@"), colors.text);
    style.replace(QStringLiteral("@hover@"), colors.hover);
    style.replace(QStringLiteral("@selected@"), colors.selected);
    return style;
}
}
