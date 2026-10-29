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
    QString accentText;
    QString hover;
    QString selected;
};

inline constexpr int PageSpacing = 18;
inline constexpr int ToolRailExpandedWidth = 240;
inline constexpr int ToolRailCollapsedWidth = 84;
inline constexpr int PanelHorizontalPadding = 20;
inline constexpr int PanelVerticalPadding = 18;
inline constexpr int PanelRadius = 8;

inline Palette palette(bool dark)
{
    if (dark) {
        return {"#111820",
                "#2c3844",
                "#e8edf2",
                "#10251f",
                "#26313c",
                "#234257"};
    }

    return {"#f3f1eb",
            "#ded8cc",
            "#1f2328",
            "#12343b",
            "#eef4ef",
            "#dcefea"};
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
    style.replace(QStringLiteral("@surface@"), colors.surface);
    style.replace(QStringLiteral("@border@"), colors.border);
    style.replace(QStringLiteral("@radius@"), QString::number(PanelRadius));
    style.replace(QStringLiteral("@text@"), colors.text);
    style.replace(QStringLiteral("@hover@"), colors.hover);
    style.replace(QStringLiteral("@selected@"), colors.selected);
    style.replace(QStringLiteral("@accent-text@"), colors.accentText);
    return style;
}
}
