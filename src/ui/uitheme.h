#pragma once

#include <QFile>
#include <QString>

namespace UiTheme
{
struct Palette
{
    QString page;
    QString surface;
    QString surfaceRaised;
    QString border;
    QString text;
    QString mutedText;
    QString accent;
    QString accentText;
    QString hover;
    QString selected;
    QString field;
    QString fieldBorder;
};

inline constexpr int PageSpacing = 18;
inline constexpr int HeaderHeight = 80;
inline constexpr int HeaderHorizontalPadding = 24;
inline constexpr int HeaderVerticalPadding = 18;
inline constexpr int ToolRailExpandedWidth = 240;
inline constexpr int ToolRailCollapsedWidth = 84;
inline constexpr int PanelHorizontalPadding = 20;
inline constexpr int PanelVerticalPadding = 18;
inline constexpr int PanelRadius = 8;

inline Palette palette(bool dark)
{
    if (dark) {
        return {"#111820",
                "#1b232c",
                "#202a34",
                "#2c3844",
                "#e8edf2",
                "#a9b7c5",
                "#75c7ad",
                "#10251f",
                "#26313c",
                "#234257",
                "#121920",
                "#3a4652"};
    }

    return {"#f3f1eb",
            "#fbfaf7",
            "#f6f3ec",
            "#ded8cc",
            "#1f2328",
            "#5e675f",
            "#7fb8a6",
            "#12343b",
            "#eef4ef",
            "#dcefea",
            "#fcfbf8",
            "#d9d4c8"};
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
    style.replace(QStringLiteral("@field@"), colors.field);
    style.replace(QStringLiteral("@field-border@"), colors.fieldBorder);
    style.replace(QStringLiteral("@selected@"), colors.selected);
    style.replace(QStringLiteral("@accent@"), colors.accent);
    style.replace(QStringLiteral("@accent-text@"), colors.accentText);
    style.replace(QStringLiteral("@muted-text@"), colors.mutedText);
    return style;
}
}
