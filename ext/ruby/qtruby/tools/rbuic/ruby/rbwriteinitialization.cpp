/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "rbwriteinitialization.h"
#include "rbwriteiconinitialization.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"
#include "uic.h"
#include "databaseinfo.h"
#include "globaldefs.h"

#include <QtCore/QTextStream>
#include <QtCore/QDebug>

#if defined(QT_BEGIN_NAMESPACE)
  QT_BEGIN_NAMESPACE
#endif

namespace {
    // Fixup an enumeration name from class Qt.
    // They are currently stored as "BottomToolBarArea" instead of "Qt::BottomToolBarArea".
    // due to MO issues. This might be fixed in the future.
    void fixQtEnumerationName(QString& name) {
        static const QString prefix = QStringLiteral("Qt::");
        if (name.indexOf(prefix) != 0)
            name.prepend(prefix);
    }
    // figure out the toolbar area of a DOM attrib list.
    // By legacy, it is stored as an integer. As of 4.3.0, it is the enumeration value.
    QString toolBarAreaStringFromDOMAttributes(const Ruby::WriteInitialization::DomPropertyMap &attributes) {
        const DomProperty *pstyle = attributes.value(QStringLiteral("toolBarArea"));
        if (!pstyle)
            return QString();

        switch (pstyle->kind()) {
        case DomProperty::Number: {
            QString area = QStringLiteral("(");
            area += QString::number(pstyle->elementNumber());
            area += QStringLiteral("), ");
            return area;
        }
        case DomProperty::Enum: {
            QString area = pstyle->elementEnum();
            fixQtEnumerationName(area);
            area += QStringLiteral(", ");
            return area;
        }
        default:
            break;
        }
        return QString();
    }

    // Write a statement to create a spacer item.
    void writeSpacerItem(const DomSpacer *node, QTextStream &output) {
        const QHash<QString, DomProperty *> properties = propertyMap(node->elementProperty());
                output << "Qt::SpacerItem.new(";

        if (properties.contains(QStringLiteral("sizeHint"))) {
            const DomSize *sizeHint = properties.value(QStringLiteral("sizeHint"))->elementSize();
            output << sizeHint->elementWidth() << ", " << sizeHint->elementHeight() << ", ";
        }

        // size type
        QString sizeType = properties.contains(QStringLiteral("sizeType"))  ?
                           properties.value(QStringLiteral("sizeType"))->elementEnum() :
                           QString::fromLatin1("Expanding");

        sizeType.replace("QSizePolicy", "Qt::SizePolicy");
        if (!sizeType.startsWith(QStringLiteral("Qt::SizePolicy::")))
            sizeType.prepend(QStringLiteral("Qt::SizePolicy::"));
        // orientation
        bool isVspacer = false;
        if (properties.contains(QStringLiteral("orientation"))) {
            const QString orientation = properties.value(QStringLiteral("orientation"))->elementEnum();
            if (orientation == QStringLiteral("Qt::Vertical") || orientation == QStringLiteral("Vertical"))  isVspacer = true;
        }

        if (isVspacer)
            output << "Qt::SizePolicy::Minimum, " << sizeType << ')';
        else
            output << sizeType << ", Qt::SizePolicy::Minimum)";
    }


    // Helper for implementing comparison functions for integers.
    int compareInt(int i1, int i2) {
        if (i1 < i2) return -1;
        if (i1 > i2) return  1;
        return  0;
    }

    // Write object.setFoo(x)
    template <class Value>
        void writeSetter(const QString &indent, const QString &varName,const QString &setter, Value v, QTextStream &str) {
            QString setFunction(setter[3].toLower());
            setFunction += setter.mid(4);
            str << indent << varName << "." << setFunction << " = " << v << "\n";
        }


    static inline bool isIconFormat44(const DomResourceIcon *i) {
        return i->hasElementNormalOff()   || i->hasElementNormalOn() ||
               i->hasElementDisabledOff() || i->hasElementDisabledOn() ||
               i->hasElementActiveOff()   || i->hasElementActiveOn() ||
               i->hasElementSelectedOff() || i->hasElementSelectedOn();
    }

    // Check on properties. Filter out empty legacy pixmap/icon properties
    // as Designer pre 4.4 used to remove missing resource references.
    // This can no longer be handled by the code as we have 'setIcon(QIcon())' as well as 'QIcon icon'
    static bool checkProperty(const QString &fileName, const DomProperty *p) {
        switch (p->kind()) {
        case DomProperty::IconSet:
            if (const DomResourceIcon *dri = p->elementIconSet()) {
                if (!isIconFormat44(dri)) {
                    if (dri->text().isEmpty())  {
                        const QString msg = QString::fromUtf8("%1: An invalid icon property '%2' was encountered.").arg(fileName).arg(p->attributeName());
                        qWarning("%s", msg.toUtf8().constData());
                        return false;
                    }
                }
            }
            break;
        case DomProperty::Pixmap:
            if (const DomResourcePixmap *drp = p->elementPixmap())
                if (drp->text().isEmpty()) {
                    const QString msg = QString::fromUtf8("%1: An invalid pixmap property '%2' was encountered.").arg(fileName).arg(p->attributeName());
                    qWarning("%s", msg.toUtf8().constData());
                    return false;
                }
            break;
        default:
            break;
        }
        return  true;
    }
}

namespace Ruby {

FontHandle::FontHandle(const DomFont *domFont) :
      m_domFont(domFont)
{
}

int FontHandle::compare(const FontHandle &rhs) const
{
    const QString family    = m_domFont->hasElementFamily()     ?     m_domFont->elementFamily() : QString();
    const QString rhsFamily = rhs.m_domFont->hasElementFamily() ? rhs.m_domFont->elementFamily() : QString();

    if (const int frc = family.compare(rhsFamily))
        return frc;

    const int pointSize    = m_domFont->hasElementPointSize()     ?     m_domFont->elementPointSize() : -1;
    const int rhsPointSize = rhs.m_domFont->hasElementPointSize() ? rhs.m_domFont->elementPointSize() : -1;

    if (const int crc = compareInt(pointSize, rhsPointSize))
        return crc;

    const int bold    = m_domFont->hasElementBold()     ? (m_domFont->elementBold()     ? 1 : 0) : -1;
    const int rhsBold = rhs.m_domFont->hasElementBold() ? (rhs.m_domFont->elementBold() ? 1 : 0) : -1;
    if (const int crc = compareInt(bold, rhsBold))
        return crc;

    const int italic    = m_domFont->hasElementItalic()     ? (m_domFont->elementItalic()     ? 1 : 0) : -1;
    const int rhsItalic = rhs.m_domFont->hasElementItalic() ? (rhs.m_domFont->elementItalic() ? 1 : 0) : -1;
    if (const int crc = compareInt(italic, rhsItalic))
        return crc;

    const int underline    = m_domFont->hasElementUnderline()     ? (m_domFont->elementUnderline()     ? 1 : 0) : -1;
    const int rhsUnderline = rhs.m_domFont->hasElementUnderline() ? (rhs.m_domFont->elementUnderline() ? 1 : 0) : -1;
    if (const int crc = compareInt(underline, rhsUnderline))
        return crc;

    const int weight    = m_domFont->hasElementWeight()     ?     m_domFont->elementWeight() : -1;
    const int rhsWeight = rhs.m_domFont->hasElementWeight() ? rhs.m_domFont->elementWeight() : -1;
    if (const int crc = compareInt(weight, rhsWeight))
        return crc;

    const int strikeOut    = m_domFont->hasElementStrikeOut()     ? (m_domFont->elementStrikeOut()     ? 1 : 0) : -1;
    const int rhsStrikeOut = rhs.m_domFont->hasElementStrikeOut() ? (rhs.m_domFont->elementStrikeOut() ? 1 : 0) : -1;
    if (const int crc = compareInt(strikeOut, rhsStrikeOut))
        return crc;

    const int kerning    = m_domFont->hasElementKerning()     ? (m_domFont->elementKerning()     ? 1 : 0) : -1;
    const int rhsKerning = rhs.m_domFont->hasElementKerning() ? (rhs.m_domFont->elementKerning() ? 1 : 0) : -1;
    if (const int crc = compareInt(kerning, rhsKerning))
        return crc;

    const int antialiasing    = m_domFont->hasElementAntialiasing()     ? (m_domFont->elementAntialiasing()     ? 1 : 0) : -1;
    const int rhsAntialiasing = rhs.m_domFont->hasElementAntialiasing() ? (rhs.m_domFont->elementAntialiasing() ? 1 : 0) : -1;
    if (const int crc = compareInt(antialiasing, rhsAntialiasing))
        return crc;

    const QString styleStrategy    = m_domFont->hasElementStyleStrategy()     ?     m_domFont->elementStyleStrategy() : QString();
    const QString rhsStyleStrategy = rhs.m_domFont->hasElementStyleStrategy() ? rhs.m_domFont->elementStyleStrategy() : QString();

    if (const int src = styleStrategy.compare(rhsStyleStrategy))
        return src;

    return 0;
}

IconHandle::IconHandle(const DomResourceIcon *domIcon) :
      m_domIcon(domIcon)
{
}

int IconHandle::compare(const IconHandle &rhs) const
{
    const QString normalOff    =     m_domIcon->hasElementNormalOff() ?     m_domIcon->elementNormalOff()->text() : QString();
    const QString rhsNormalOff = rhs.m_domIcon->hasElementNormalOff() ? rhs.m_domIcon->elementNormalOff()->text() : QString();
    if (const int comp = normalOff.compare(rhsNormalOff))
        return comp;

    const QString normalOn    =     m_domIcon->hasElementNormalOn() ?     m_domIcon->elementNormalOn()->text() : QString();
    const QString rhsNormalOn = rhs.m_domIcon->hasElementNormalOn() ? rhs.m_domIcon->elementNormalOn()->text() : QString();
    if (const int comp = normalOn.compare(rhsNormalOn))
        return comp;

    const QString disabledOff    =     m_domIcon->hasElementDisabledOff() ?     m_domIcon->elementDisabledOff()->text() : QString();
    const QString rhsDisabledOff = rhs.m_domIcon->hasElementDisabledOff() ? rhs.m_domIcon->elementDisabledOff()->text() : QString();
    if (const int comp = disabledOff.compare(rhsDisabledOff))
        return comp;

    const QString disabledOn    =     m_domIcon->hasElementDisabledOn() ?     m_domIcon->elementDisabledOn()->text() : QString();
    const QString rhsDisabledOn = rhs.m_domIcon->hasElementDisabledOn() ? rhs.m_domIcon->elementDisabledOn()->text() : QString();
    if (const int comp = disabledOn.compare(rhsDisabledOn))
        return comp;

    const QString activeOff    =     m_domIcon->hasElementActiveOff() ?     m_domIcon->elementActiveOff()->text() : QString();
    const QString rhsActiveOff = rhs.m_domIcon->hasElementActiveOff() ? rhs.m_domIcon->elementActiveOff()->text() : QString();
    if (const int comp = activeOff.compare(rhsActiveOff))
        return comp;

    const QString activeOn    =     m_domIcon->hasElementActiveOn() ?     m_domIcon->elementActiveOn()->text() : QString();
    const QString rhsActiveOn = rhs.m_domIcon->hasElementActiveOn() ? rhs.m_domIcon->elementActiveOn()->text() : QString();
    if (const int comp = activeOn.compare(rhsActiveOn))
        return comp;

    const QString selectedOff    =     m_domIcon->hasElementSelectedOff() ?     m_domIcon->elementSelectedOff()->text() : QString();
    const QString rhsSelectedOff = rhs.m_domIcon->hasElementSelectedOff() ? rhs.m_domIcon->elementSelectedOff()->text() : QString();
    if (const int comp = selectedOff.compare(rhsSelectedOff))
        return comp;

    const QString selectedOn    =     m_domIcon->hasElementSelectedOn() ?     m_domIcon->elementSelectedOn()->text() : QString();
    const QString rhsSelectedOn = rhs.m_domIcon->hasElementSelectedOn() ? rhs.m_domIcon->elementSelectedOn()->text() : QString();
    if (const int comp = selectedOn.compare(rhsSelectedOn))
        return comp;
    // Pre 4.4 Legacy
    if (const int comp = m_domIcon->text().compare(rhs.m_domIcon->text()))
        return comp;

    return 0;
}


#if defined(Q_OS_MAC) && defined(Q_CC_GNU) && (__GNUC__ == 3 && __GNUC_MINOR__ == 3)
inline uint qHash(const SizePolicyHandle &handle) { return qHash(handle.m_domSizePolicy); }
inline uint qHash(const FontHandle &handle) { return qHash(handle.m_domFont); }
inline uint qHash(const IconHandle &handle) { return qHash(handle.m_domIcon); }
#endif

SizePolicyHandle::SizePolicyHandle(const DomSizePolicy *domSizePolicy) :
    m_domSizePolicy(domSizePolicy)
{
}

int SizePolicyHandle::compare(const SizePolicyHandle &rhs) const
{

    const int hSizeType    = m_domSizePolicy->hasElementHSizeType()     ? m_domSizePolicy->elementHSizeType()     : -1;
    const int rhsHSizeType = rhs.m_domSizePolicy->hasElementHSizeType() ? rhs.m_domSizePolicy->elementHSizeType() : -1;
    if (const int crc = compareInt(hSizeType, rhsHSizeType))
        return crc;

    const int vSizeType    = m_domSizePolicy->hasElementVSizeType()     ? m_domSizePolicy->elementVSizeType()     : -1;
    const int rhsVSizeType = rhs.m_domSizePolicy->hasElementVSizeType() ? rhs.m_domSizePolicy->elementVSizeType() : -1;
    if (const int crc = compareInt(vSizeType, rhsVSizeType))
        return crc;

    const int hStretch    =  m_domSizePolicy->hasElementHorStretch()     ? m_domSizePolicy->elementHorStretch()     : -1;
    const int rhsHStretch =  rhs.m_domSizePolicy->hasElementHorStretch() ? rhs.m_domSizePolicy->elementHorStretch() : -1;
    if (const int crc = compareInt(hStretch, rhsHStretch))
        return crc;

    const int vStretch    =  m_domSizePolicy->hasElementVerStretch()     ? m_domSizePolicy->elementVerStretch()     : -1;
    const int rhsVStretch =  rhs.m_domSizePolicy->hasElementVerStretch() ? rhs.m_domSizePolicy->elementVerStretch() : -1;
    if (const int crc = compareInt(vStretch, rhsVStretch))
        return crc;

    const QString attributeHSizeType    = m_domSizePolicy->hasAttributeHSizeType()     ? m_domSizePolicy->attributeHSizeType()     : QString();
    const QString rhsAttributeHSizeType = rhs.m_domSizePolicy->hasAttributeHSizeType() ? rhs.m_domSizePolicy->attributeHSizeType() : QString();

    if (const int hrc = attributeHSizeType.compare(rhsAttributeHSizeType))
        return hrc;

    const QString attributeVSizeType    = m_domSizePolicy->hasAttributeVSizeType()     ? m_domSizePolicy->attributeVSizeType()     : QString();
    const QString rhsAttributeVSizeType = rhs.m_domSizePolicy->hasAttributeVSizeType() ? rhs.m_domSizePolicy->attributeVSizeType() : QString();

    return attributeVSizeType.compare(rhsAttributeVSizeType);
}

// ---  WriteInitialization: LayoutDefaultHandler

WriteInitialization::LayoutDefaultHandler::LayoutDefaultHandler()
{
    qFill(m_state, m_state + NumProperties, 0u);
    qFill(m_defaultValues, m_defaultValues + NumProperties, 0);
}



void WriteInitialization::LayoutDefaultHandler::acceptLayoutDefault(DomLayoutDefault *node)
{
    if (!node)
        return;
    if (node->hasAttributeMargin()) {
        m_state[Margin] |= HasDefaultValue;
        m_defaultValues[Margin] = node->attributeMargin();
    }
    if (node->hasAttributeSpacing()) {
        m_state[Spacing] |= HasDefaultValue;
        m_defaultValues[Spacing]  = node->attributeSpacing();
    }
}

void WriteInitialization::LayoutDefaultHandler::acceptLayoutFunction(DomLayoutFunction *node)
{
    if (!node)
        return;
    if (node->hasAttributeMargin()) {
        m_state[Margin]     |= HasDefaultFunction;
        m_functions[Margin] =  node->attributeMargin();
        m_functions[Margin] += QStringLiteral("()");
    }
    if (node->hasAttributeSpacing()) {
        m_state[Spacing]     |= HasDefaultFunction;
        m_functions[Spacing] =  node->attributeSpacing();
        m_functions[Spacing] += QStringLiteral("()");
    }
}

void WriteInitialization::LayoutDefaultHandler::writeProperty(int p, const QString &indent, const QString &objectName,
                                                              const DomPropertyMap &properties, const QString &propertyName, const QString &setter,
                                                              int /*defaultStyleValue*/, bool suppressDefault, QTextStream &str) const
{
    // User value
    const DomPropertyMap::const_iterator mit = properties.constFind(propertyName);
    const bool found = mit != properties.constEnd();
    if (found) {
        const int value = mit.value()->elementNumber();
        // Emulate the pre 4.3 behaviour: The value form default value was only used to determine
        // the default value, layout properties were always written
        const bool useLayoutFunctionPre43 = !suppressDefault && (m_state[p] == (HasDefaultFunction|HasDefaultValue)) && value == m_defaultValues[p];
        if (!useLayoutFunctionPre43) {
            writeSetter(indent, objectName, setter, value, str);
            return;
        }
    }
    if (suppressDefault)
        return;
    // get default
    if (m_state[p] & HasDefaultFunction) {
        writeSetter(indent, objectName, setter, m_functions[p], str);
        return;
    }
    if (m_state[p] & HasDefaultValue) {
        writeSetter(indent, objectName, setter, m_defaultValues[p], str);
    }
    return;
}


void WriteInitialization::LayoutDefaultHandler::writeProperties(const QString &indent, const QString &varName,
                                                                const DomPropertyMap &properties, int marginType,
                                                                bool suppressMarginDefault,
                                                                QTextStream &str) const {
    // Write out properties and ignore the ones found in
    // subsequent writing of the property list.
    int defaultSpacing = marginType == WriteInitialization::Use43UiFile ? -1 : 6;
    writeProperty(Spacing, indent, varName, properties, QStringLiteral("spacing"), QStringLiteral("setSpacing"),
                  defaultSpacing, false, str);
    // We use 9 as TopLevelMargin, since Designer seem to always use 9.
    static const int layoutmargins[4] = {-1, 9, 9, 0};
    writeProperty(Margin,  indent, varName, properties, QStringLiteral("margin"),  QStringLiteral("setMargin"),
                  layoutmargins[marginType], suppressMarginDefault, str);
}

// ---  WriteInitialization
WriteInitialization::WriteInitialization(Uic *uic) :
      m_uic(uic),
      m_driver(uic->driver()), m_output(uic->output()), m_option(uic->option()),
      m_layoutMarginType(TopLevelMargin),
      m_delayedOut(&m_delayedInitialization, QIODevice::WriteOnly),
      m_refreshOut(&m_refreshInitialization, QIODevice::WriteOnly),
      m_actionOut(&m_delayedActionInitialization, QIODevice::WriteOnly),
      m_layoutWidget(false)
{
}

void WriteInitialization::acceptUI(DomUI *node)
{
    m_registeredImages.clear();
    m_actionGroupChain.push(0);
    m_widgetChain.push(0);
    m_layoutChain.push(0);

    acceptLayoutDefault(node->elementLayoutDefault());
    acceptLayoutFunction(node->elementLayoutFunction());

    if (node->elementCustomWidgets())
        TreeWalker::acceptCustomWidgets(node->elementCustomWidgets());

    if (node->elementImages())
        TreeWalker::acceptImages(node->elementImages());

    m_stdsetdef = true;
    if (node->hasAttributeStdSetDef())
        m_stdsetdef = node->attributeStdSetDef();

    const QString className = node->elementClass() + m_option.postfix;
    m_generatedClass = className;

    m_mainWidget = m_driver->findOrInsertWidget(node->elementWidget());
    m_registeredWidgets.insert(m_mainWidget, node->elementWidget()); // register the main widget

	m_mainWidget = m_mainWidget.mid(0, 1).toLower() + m_mainWidget.mid(1);

    const QString widgetClassName = node->elementWidget()->attributeClass();

    m_output << m_option.indent << "def " << "setupUi(" << m_mainWidget << ")\n";

    const QStringList connections = m_uic->databaseInfo()->connections();
    for (int i=0; i<connections.size(); ++i) {
        QString connection = connections.at(i);

        if (connection == QStringLiteral("(default)"))
            continue;

        QString varConn = QStringLiteral("@") + connection + QStringLiteral("Connection");
        m_output << m_option.indent << varConn << " = Qt::SqlDatabase.database(" << fixString(connection, m_option.indent) << ")\n";
    }

    acceptWidget(node->elementWidget());

    for (int i=0; i<m_buddies.size(); ++i) {
        const Buddy &b = m_buddies.at(i);
        QString name(b.objName);
        name.replace("@", "");
        QString buddyName(b.buddy);
        buddyName.replace("@", "");

        if (!m_registeredWidgets.contains(name) && !m_registeredWidgets.contains(name)) {
            fprintf(stderr, "'@%s' isn't a valid widget\n", name.toLocal8Bit().constData());
            continue;
        } else if (!m_registeredWidgets.contains(b.buddy) && !m_registeredWidgets.contains(buddyName)) {
            fprintf(stderr, "'@%s' isn't a valid widget\n", buddyName.toLocal8Bit().constData());
            continue;
        }

        m_output << m_option.indent << "@" << name << ".buddy = @" << buddyName << "\n";
    }

    if (node->elementTabStops())
        acceptTabStops(node->elementTabStops());

    if (m_delayedActionInitialization.size())
        m_output << "\n" << m_delayedActionInitialization;

    m_output << "\n" << m_option.indent << "retranslateUi(" << m_mainWidget << ")\n";

    if (node->elementConnections())
        acceptConnections(node->elementConnections());

    if (!m_delayedInitialization.isEmpty())
        m_output << "\n" << m_delayedInitialization << "\n";

    if (m_option.autoConnection)
        m_output << "\n" << m_option.indent << "Qt::MetaObject.connectSlotsByName(" << m_mainWidget << ")\n";

    m_output << m_option.indent << "end # setupUi\n\n";

    m_output << m_option.indent << "def " << "setup_ui(" << m_mainWidget << ")\n";
    m_output << m_option.indent << "    setupUi(" << m_mainWidget << ")\n";
    m_output << m_option.indent << "end\n\n";

    m_output << m_option.indent << "def " << "retranslateUi(" << m_mainWidget << ")\n"
           << m_refreshInitialization
           << m_option.indent << "end # retranslateUi\n\n";

    m_output << m_option.indent << "def " << "retranslate_ui(" << m_mainWidget << ")\n";
    m_output << m_option.indent << "    retranslateUi(" << m_mainWidget << ")\n";
    m_output << m_option.indent << "end\n\n";

    m_layoutChain.pop();
    m_widgetChain.pop();
    m_actionGroupChain.pop();
}

void WriteInitialization::acceptWidget(DomWidget *node)
{
    m_layoutMarginType = m_widgetChain.count() == 1 ? TopLevelMargin : ChildMargin;
    const QString className = node->attributeClass();
    QString varName = m_driver->findOrInsertWidget(node);
    m_registeredWidgets.insert(varName, node); // register the current widget

    varName = varName.mid(0, 1).toLower() + varName.mid(1);

    if (m_layoutMarginType == ChildMargin) {
        varName.prepend("@");
    }

    QString parentWidget, parentClass;
    if (m_widgetChain.top()) {
        parentWidget = m_driver->findOrInsertWidget(m_widgetChain.top());
        parentWidget = parentWidget.mid(0, 1).toLower() + parentWidget.mid(1);
        if (m_widgetChain.count() != 2) {
            parentWidget.prepend("@");
        }
        parentClass = m_widgetChain.top()->attributeClass();
    }

    const QString savedParentWidget = parentWidget;

    if (m_uic->isContainer(parentClass) || m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("Q3ToolBar")))
        parentWidget.clear();

    if (m_widgetChain.size() != 1) {
        m_output << m_option.indent << varName << " = " << m_driver->rubyClassName(m_uic->customWidgetsInfo()->realClassName(className)) << ".new(" << parentWidget << ")\n";
    }

    parentWidget = savedParentWidget;

    if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QComboBox"))) {
        initializeComboBox(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QListWidget"))) {
        initializeListWidget(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QTreeWidget"))) {
        initializeTreeWidget(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QTableWidget"))) {
        initializeTableWidget(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("Q3ListBox"))) {
        initializeQ3ListBox(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("Q3ListView"))) {
        initializeQ3ListView(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("Q3IconView"))) {
        initializeQ3IconView(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("Q3Table"))) {
        initializeQ3Table(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("Q3DataTable"))) {
        initializeQ3SqlDataTable(node);
    } else if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("Q3DataBrowser"))) {
        initializeQ3SqlDataBrowser(node);
    }

    if (m_uic->isButton(className)) {
        const DomPropertyMap attributes = propertyMap(node->elementAttribute());
        if (const DomProperty *prop = attributes.value(QStringLiteral("buttonGroup"))) {
            const QString groupName = toString(prop->elementString());
            if (!m_buttonGroups.contains(groupName)) {
                m_buttonGroups.insert(groupName, m_driver->findOrInsertName(groupName));
                const QString g = QString("@") + m_buttonGroups.value(groupName);
                m_output << m_option.indent << g << " = Qt::ButtonGroup.new(" << m_generatedClass << ")\n";
            }

            const QString g = QString("@") + m_buttonGroups.value(groupName);
            m_output << m_option.indent << g << ".addButton(" << varName << ")\n";
        }
    }

    writeProperties(varName, className, node->elementProperty());

    if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QMenu")) && parentWidget.size()) {
        initializeMenu(node, parentWidget);
    }

    if (node->elementLayout().isEmpty())
        m_layoutChain.push(0);

    m_layoutWidget = false;
    if (className == QStringLiteral("QWidget") && !node->hasAttributeNative()) {
        if (m_widgetChain.top()
            && m_widgetChain.top()->attributeClass() != QStringLiteral("QMainWindow")
            && !m_uic->isContainer(m_widgetChain.top()->attributeClass()))
            m_layoutWidget = true;
    }
    m_widgetChain.push(node);
    m_layoutChain.push(0);
    TreeWalker::acceptWidget(node);
    m_layoutChain.pop();
    m_widgetChain.pop();
    m_layoutWidget = false;

    const DomPropertyMap attributes = propertyMap(node->elementAttribute());

    QString title = QStringLiteral("Page");
    if (const DomProperty *ptitle = attributes.value(QStringLiteral("title"))) {
        title = toString(ptitle->elementString());
    }

    QString label = QStringLiteral("Page");
    if (const DomProperty *plabel = attributes.value(QStringLiteral("label"))) {
        label = toString(plabel->elementString());
    }

    int id = -1;
    if (const DomProperty *pid = attributes.value(QStringLiteral("id"))) {
        id = pid->elementNumber();
    }

    if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("QMainWindow"))
            || m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("Q3MainWindow"))) {

        if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QMenuBar"))) {
            if (!m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("Q3MainWindow")))
                m_output << m_option.indent << parentWidget << ".setMenuBar(" << varName <<")\n";
        } else if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QToolBar"))) {
            m_output << m_option.indent << parentWidget << ".addToolBar("
                     << toolBarAreaStringFromDOMAttributes(attributes) << varName << ")\n";

            if (const DomProperty *pbreak = attributes.value(QStringLiteral("toolBarBreak"))) {
                if (pbreak->elementBool() == QStringLiteral("true")) {
                    m_output << m_option.indent << parentWidget << ".insertToolBarBreak(" <<  varName << ")\n";
                }
            }

        } else if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QDockWidget"))) {
            QString area;
            if (DomProperty *pstyle = attributes.value(QStringLiteral("dockWidgetArea"))) {
                area += QStringLiteral("(");
                area += QString::number(pstyle->elementNumber());
                area += QStringLiteral("), ");
            }

            m_output << m_option.indent << parentWidget << ".addDockWidget(" << area << varName << ")\n";
        } else if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QStatusBar"))) {
            m_output << m_option.indent << parentWidget << ".statusBar = " << varName << "\n";
        } else if (className == QStringLiteral("QWidget")) {
            m_output << m_option.indent << parentWidget << ".centralWidget = " << varName << "\n";
        }
    }

    // Check for addPageMethod of a custom plugin first
    const QString addPageMethod = m_uic->customWidgetsInfo()->customWidgetAddPageMethod(parentClass);
    if (!addPageMethod.isEmpty()) {
        m_output << m_option.indent << parentWidget << "." << addPageMethod << '(' << varName << ")\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("QStackedWidget"))) {
        m_output << m_option.indent << parentWidget << ".addWidget(" << varName << ")\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("QToolBar"))) {
        m_output << m_option.indent << parentWidget << ".addWidget(" << varName << ")\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("Q3WidgetStack"))) {
        m_output << m_option.indent << parentWidget << ".addWidget(" << varName << ", " << id << ")\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("QDockWidget"))) {
        m_output << m_option.indent << parentWidget << ".setWidget(" << varName << ")\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("QScrollArea"))) {
        m_output << m_option.indent << parentWidget << ".setWidget(" << varName << ")\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("QSplitter"))) {
        m_output << m_option.indent << parentWidget << ".addWidget(" << varName << ")\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("QMdiArea"))) {
        m_output << m_option.indent << parentWidget << ".addSubWindow(" << varName << ")\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("QWorkspace"))) {
        m_output << m_option.indent << parentWidget << ".addWindow(" << varName << ")\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("QWizard"))) {
        m_output << m_option.indent << parentWidget << ".addPage(" << varName << ")\n";
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("QToolBox"))) {
        QString icon;
        if (DomProperty *picon = attributes.value(QStringLiteral("icon"))) {
            icon += QStringLiteral(", ") ;
            icon += iconCall(picon);
        }

        m_output << m_option.indent << parentWidget << ".addItem(" << varName << icon << ", " << trCall(label) << ")\n";

        m_refreshOut << m_option.indent << parentWidget << ".setItemText("
                   << parentWidget << ".indexOf(" << varName << "), " << trCall(label) << ")\n";

        if (DomProperty *ptoolTip = attributes.value(QStringLiteral("toolTip"))) {
            m_refreshOut << m_option.indent << parentWidget << ".setItemToolTip("
                       << parentWidget << ".indexOf(" << varName << "), " << trCall(ptoolTip->elementString()) << ")\n";
        }
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("QTabWidget"))) {
        QString icon;
        if (const DomProperty *picon = attributes.value(QStringLiteral("icon"))) {
            icon += QStringLiteral(", ");
            icon += iconCall(picon);
        }

        m_output << m_option.indent << parentWidget << ".addTab(" << varName << icon << ", " << trCall(title) << ")\n";

        m_refreshOut << m_option.indent << parentWidget << ".setTabText("
                   << parentWidget << ".indexOf(" << varName << "), " << trCall(title) << ")\n";

        if (const DomProperty *ptoolTip = attributes.value(QStringLiteral("toolTip"))) {
            m_refreshOut << m_option.indent << parentWidget << ".setTabToolTip("
                       << parentWidget << ".indexOf(" << varName << "), " << trCall(ptoolTip->elementString()) << ")\n";
        }
    } else if (m_uic->customWidgetsInfo()->extends(parentClass, QStringLiteral("Q3Wizard"))) {
        m_output << m_option.indent << parentWidget << ".addPage(" << varName << ", " << trCall(title) << ")\n";

        m_refreshOut << m_option.indent << parentWidget << ".setTitle("
                   << varName << ", " << trCall(title) << ")\n";

    }

    if (node->elementLayout().isEmpty())
        m_layoutChain.pop();

    const QStringList zOrder = node->elementZOrder();
    for (int i = 0; i < zOrder.size(); ++i) {
        const QString name = zOrder.at(i);

        if (!m_registeredWidgets.contains(name)) {
            fprintf(stderr, "'%s' isn't a valid widget\n", name.toLocal8Bit().constData());
            continue;
        }

        if (name.isEmpty()) {
            continue;
        }

        m_output << m_option.indent << name << ".raise()\n";
    }
}

void WriteInitialization::acceptLayout(DomLayout *node)
{
    QString className = node->attributeClass();
    QString varName = toRubyIdentifier(m_driver->findOrInsertLayout(node));

    const DomPropertyMap properties = propertyMap(node->elementProperty());
    const bool oldLayoutProperties = properties.constFind(QStringLiteral("margin")) != properties.constEnd();

    bool isGroupBox = false;

    if (m_widgetChain.top()) {
        const QString parentWidget = m_widgetChain.top()->attributeClass();

        if (!m_layoutChain.top() && (m_uic->customWidgetsInfo()->extends(parentWidget, QStringLiteral("Q3GroupBox"))
                        || m_uic->customWidgetsInfo()->extends(parentWidget, QStringLiteral("Q3ButtonGroup")))) {
            const QString parent = toRubyIdentifier(m_driver->findOrInsertWidget(m_widgetChain.top()));

            isGroupBox = true;
            // special case for group box

            m_output << m_option.indent << parent << ".setColumnLayout(0, Qt::Vertical)\n";
            QString objectName = parent;
            objectName += QStringLiteral(".layout");
            int marginType = Use43UiFile;
            if (oldLayoutProperties)
                marginType = m_layoutMarginType;

            m_LayoutDefaultHandler.writeProperties(m_option.indent, 
                                    objectName, properties, marginType, false, m_output);
        }
    }

    m_output << m_option.indent << varName << " = " << m_driver->rubyClassName(className) << ".new(";

    if (!m_layoutChain.top() && !isGroupBox) {
        QString parentWidget = m_driver->findOrInsertWidget(m_widgetChain.top());
        parentWidget = parentWidget.mid(0, 1).toLower() + parentWidget.mid(1);
        if (m_widgetChain.count() != 2) {
            parentWidget.prepend("@");
        }
        m_output << parentWidget;
    }

    m_output << ")\n";

    if (isGroupBox) {
        const QString tempName = m_driver->unique(QStringLiteral("boxlayout"));
        QString widget = m_driver->findOrInsertWidget(m_widgetChain.top());
        widget = widget.mid(0, 1).toLower() + widget.mid(1);
        widget.prepend("@");
      
        m_output << m_option.indent << "" << tempName << " = " <<
                    widget << ".layout\n";
        m_output << m_option.indent << "if " << tempName << "\n";
        m_output << m_option.indent << "    " << tempName << ".addLayout(" << varName << ")\n";
        m_output << m_option.indent << "end " << "\n";
    }

    if (isGroupBox) {
        m_output << m_option.indent << varName << ".alignment = Qt::AlignTop\n";
    } else {
       // Suppress margin on a read child layout
        const bool suppressMarginDefault = m_layoutChain.top();
        int marginType = Use43UiFile;
        if (oldLayoutProperties)
            marginType = m_layoutMarginType;
        m_LayoutDefaultHandler.writeProperties(m_option.indent, varName, properties, marginType, suppressMarginDefault, m_output);
    }

    m_layoutMarginType = SubLayoutMargin;

    DomPropertyList propList = node->elementProperty();
    if (m_layoutWidget) {
        bool left, top, right, bottom;
        left = top = right = bottom = false;
        for (int i = 0; i < propList.size(); ++i) {
            const DomProperty *p = propList.at(i);
            const QString propertyName = p->attributeName();
            if (propertyName == QStringLiteral("leftMargin") && p->kind() == DomProperty::Number)
                left = true;
            else if (propertyName == QStringLiteral("topMargin") && p->kind() == DomProperty::Number)
                top = true;
            else if (propertyName == QStringLiteral("rightMargin") && p->kind() == DomProperty::Number)
                right = true;
            else if (propertyName == QStringLiteral("bottomMargin") && p->kind() == DomProperty::Number)
                bottom = true;
        }
        if (!left) {
            DomProperty *p = new DomProperty();
            p->setAttributeName(QStringLiteral("leftMargin"));
            p->setElementNumber(0);
            propList.append(p);
        }
        if (!top) {
            DomProperty *p = new DomProperty();
            p->setAttributeName(QStringLiteral("topMargin"));
            p->setElementNumber(0);
            propList.append(p);
        }
        if (!right) {
            DomProperty *p = new DomProperty();
            p->setAttributeName(QStringLiteral("rightMargin"));
            p->setElementNumber(0);
            propList.append(p);
        }
        if (!bottom) {
            DomProperty *p = new DomProperty();
            p->setAttributeName(QStringLiteral("bottomMargin"));
            p->setElementNumber(0);
            propList.append(p);
        }
        m_layoutWidget = false;
    }

    writeProperties(varName, className, propList, WritePropertyIgnoreMargin|WritePropertyIgnoreSpacing);


    m_layoutChain.push(node);
    TreeWalker::acceptLayout(node);
    m_layoutChain.pop();
}

void WriteInitialization::acceptSpacer(DomSpacer *node)
{
    m_output << m_option.indent << "@" << m_driver->findOrInsertSpacer(node) << " = ";
    writeSpacerItem(node, m_output);
    m_output << "\n";
}

void WriteInitialization::acceptLayoutItem(DomLayoutItem *node)
{
    TreeWalker::acceptLayoutItem(node);

    DomLayout *layout = m_layoutChain.top();

    if (!layout)
        return;

    const QString layoutName = toRubyIdentifier(m_driver->findOrInsertLayout(layout));
    const QString itemName = toRubyIdentifier(m_driver->findOrInsertLayoutItem(node));

    QString addArgs;
    QString methodPrefix = QStringLiteral("add"); //Consistent API-design galore!
    if (layout->attributeClass() == QStringLiteral("QGridLayout")) {
        const int row = node->attributeRow();
        const int col = node->attributeColumn();

        int rowSpan = 1;
        if (node->hasAttributeRowSpan())
            rowSpan = node->attributeRowSpan();

        int colSpan = 1;
        if (node->hasAttributeColSpan())
            colSpan = node->attributeColSpan();

        addArgs = QString::fromLatin1("%1, %2, %3, %4, %5").arg(itemName).arg(row).arg(col).arg(rowSpan).arg(colSpan);
    } else {
        if (layout->attributeClass() == QStringLiteral("QFormLayout")) {
            methodPrefix = QStringLiteral("set");
            const int row = node->attributeRow();
            const QString role = node->attributeColumn() == 0 ? QStringLiteral("Qt::FormLayout::LabelRole") : QStringLiteral("Qt::FormLayout::FieldRole");
            addArgs = QString::fromLatin1("%1, %2, %3").arg(row).arg(role).arg(itemName);
        } else {
            addArgs = itemName;
        }
    }

    // figure out "add" method
    m_output << "\n" << m_option.indent << layoutName << ".";
    switch (node->kind()) {
    case DomLayoutItem::Widget:
        m_output << methodPrefix << "Widget(" <<  addArgs;
        break;
    case DomLayoutItem::Layout:
        m_output <<  methodPrefix << "Layout(" << addArgs;
        break;
    case DomLayoutItem::Spacer:
        m_output << methodPrefix << "Item(" << addArgs;
        break;
    case DomLayoutItem::Unknown:
        Q_ASSERT( 0 );
        break;
    }
    m_output << ")\n\n";
}

void WriteInitialization::acceptActionGroup(DomActionGroup *node)
{
    QString actionName = toRubyIdentifier(m_driver->findOrInsertActionGroup(node));

    QString varName = m_driver->findOrInsertWidget(m_widgetChain.top());
    varName = varName.mid(0, 1).toLower() + varName.mid(1);
    if (m_widgetChain.count() > 2) {
        varName.prepend("@");
    }

    if (m_actionGroupChain.top())
        varName = m_driver->findOrInsertActionGroup(m_actionGroupChain.top());

    m_output << m_option.indent << actionName << " = Qt:ActionGroup.new(" << varName << ")\n";
    writeProperties(actionName, QStringLiteral("QActionGroup"), node->elementProperty());

    m_actionGroupChain.push(node);
    TreeWalker::acceptActionGroup(node);
    m_actionGroupChain.pop();
}

void WriteInitialization::acceptAction(DomAction *node)
{
    if (node->hasAttributeMenu())
        return;

    const QString actionName = toRubyIdentifier(m_driver->findOrInsertAction(node));

    m_registeredActions.insert(actionName, node);
    QString varName = m_driver->findOrInsertWidget(m_widgetChain.top());
    varName = varName.mid(0, 1).toLower() + varName.mid(1);
    if (m_widgetChain.count() > 2) {
        varName.prepend("@");
    }

    if (m_actionGroupChain.top()) {
        varName = toRubyIdentifier(m_driver->findOrInsertActionGroup(m_actionGroupChain.top()));
    }

    m_output << m_option.indent << actionName << " = Qt::Action.new(" << varName << ")\n";
    writeProperties(actionName, QStringLiteral("QAction"), node->elementProperty());
}

void WriteInitialization::acceptActionRef(DomActionRef *node)
{
    QString actionName = node->attributeName();
    QString rubyActionName = toRubyIdentifier(node->attributeName());
    bool isSeparator = actionName == QStringLiteral("separator");
    bool isMenu = false;

    QString varName = m_driver->findOrInsertWidget(m_widgetChain.top());
    varName = varName.mid(0, 1).toLower() + varName.mid(1);
    if (m_widgetChain.count() > 2) {
        varName.prepend("@");
    }

    if (actionName.isEmpty() || !m_widgetChain.top()) {
        return;
    } else if (m_driver->actionGroupByName(actionName)) {
        return;
    } else if (DomWidget *w = m_driver->widgetByName(actionName)) {
        isMenu = m_uic->isMenu(w->attributeClass());
        bool inQ3ToolBar = m_uic->customWidgetsInfo()->extends(m_widgetChain.top()->attributeClass(), QStringLiteral("Q3ToolBar"));
        if (!isMenu && inQ3ToolBar) {
            m_actionOut << m_option.indent << rubyActionName << ".parent = " << varName << "\n";
            return;
        }
    } else if (!(m_driver->actionByName(actionName) || isSeparator)) {
        fprintf(stderr, "Warning: action `%s' not declared\n", actionName.toLocal8Bit().constData());
        return;
    }

    if (m_widgetChain.top() && isSeparator) {
        // separator is always reserved!
        m_actionOut << m_option.indent << varName << ".addSeparator()\n";
        return;
    }

    if (isMenu)
        rubyActionName += QStringLiteral(".menuAction()");

    m_actionOut << m_option.indent << varName << ".addAction(" << rubyActionName << ")\n";
}

void WriteInitialization::writeProperties(const QString &varName,
                                          const QString &className,
                                          const DomPropertyList &lst,
                                          unsigned flags)
{
    bool isTopLevel = m_widgetChain.count() == 1;

    if (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QAxWidget"))) {
        DomPropertyMap properties = propertyMap(lst);
        if (properties.contains(QStringLiteral("control"))) {
            DomProperty *p = properties.value(QStringLiteral("control"));
            m_output << m_option.indent << varName << ".setControl("
                   << fixString(toString(p->elementString()), m_option.indent) << ")\n";
        }
    }

    DomWidget *buttonGroupWidget = findWidget(QStringLiteral("Q3ButtonGroup"));

    QString indent;
    if (!m_widgetChain.top()) {
        indent = QStringLiteral("    ");
        m_output << m_option.indent << "if " << varName << ".objectName.nil?\n";
    }

    if (varName.startsWith("@")) {
        m_output << m_option.indent << indent << varName << ".objectName = " << fixString(varName.mid(1), m_option.indent) << "\n";
    } else {
        m_output << m_option.indent << indent << varName << ".objectName = " << fixString(varName.mid(0,1).toLower() + varName.mid(1), m_option.indent) << "\n";
    }

    if (!m_widgetChain.top()) {
        indent = QStringLiteral("    ");
        m_output << m_option.indent << "end\n";
    }

    int leftMargin, topMargin, rightMargin, bottomMargin;
    leftMargin = topMargin = rightMargin = bottomMargin = -1;
    bool frameShadowEncountered = false;

    for (int i=0; i<lst.size(); ++i) {
        DomProperty *p = lst.at(i);
        if (!checkProperty(m_option.inputFile, p))
            continue;
        QString propertyName = p->attributeName();
        QString propertyValue;

        // special case for the property `geometry': Do not use position
        if (isTopLevel && propertyName == QStringLiteral("geometry") && p->elementRect()) {
            const DomRect *r = p->elementRect();
            m_output << m_option.indent << varName << ".resize(" << r->elementWidth() << ", " << r->elementHeight() << ")\n";
            continue;
        } else if (propertyName == QStringLiteral("buttonGroupId") && buttonGroupWidget) { // Q3ButtonGroup support
            m_output << m_option.indent << m_driver->findOrInsertWidget(buttonGroupWidget) << ".insert("
                   << varName << ", " << p->elementNumber() << ")\n";
            continue;
        } else if (propertyName == QStringLiteral("currentRow") // QListWidget::currentRow
                    && m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QListWidget"))) {
            m_delayedOut << m_option.indent << varName << ".setCurrentRow("
                       << p->elementNumber() << ")\n";
            continue;
        } else if (propertyName == QStringLiteral("currentIndex") // set currentIndex later
                    && (m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QComboBox"))
                    || m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QStackedWidget"))
                    || m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QTabWidget"))
                    || m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QToolBox")))) {
            m_delayedOut << m_option.indent << varName << ".setCurrentIndex("
                       << p->elementNumber() << ")\n";
            continue;
        } else if (propertyName == QStringLiteral("tabSpacing")
                    && m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QToolBox"))) {
            m_delayedOut << m_option.indent << varName << ".layout().setSpacing("
                       << p->elementNumber() << ")\n";
            continue;
        } else if (propertyName == QStringLiteral("control") // ActiveQt support
                    && m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QAxWidget"))) {
            // already done ;)
            continue;
        } else if (propertyName == QStringLiteral("database")
                    && p->elementStringList()) {
            // Sql support
            continue;
        } else if (propertyName == QStringLiteral("frameworkCode")
                    && p->kind() == DomProperty::Bool) {
            // Sql support
            continue;
        } else if (propertyName == QStringLiteral("orientation")
                    && m_uic->customWidgetsInfo()->extends(className, QStringLiteral("Line"))) {
            // Line support
            QString shape = QStringLiteral("Qt::Frame::HLine");
            if (p->elementEnum() == QStringLiteral("Qt::Vertical"))
                shape = QStringLiteral("Qt::Frame::VLine");

            m_output << m_option.indent << varName << ".setFrameShape(" << shape << ")\n";
            // QFrame Default is 'Plain'. Make the line 'Sunken' unless otherwise specified
            if (!frameShadowEncountered)
                m_output << m_option.indent << varName << ".setFrameShadow(Qt::Frame::Sunken)\n";
            continue;
        } else if ((flags & WritePropertyIgnoreMargin)  && propertyName == QStringLiteral("margin")) {
            continue;
        } else if ((flags & WritePropertyIgnoreSpacing) && propertyName == QStringLiteral("spacing")) {
            continue;
        } else if (propertyName == QStringLiteral("leftMargin") && p->kind() == DomProperty::Number) {
            leftMargin = p->elementNumber();
            continue;
        } else if (propertyName == QStringLiteral("topMargin") && p->kind() == DomProperty::Number) {
            topMargin = p->elementNumber();
            continue;
        } else if (propertyName == QStringLiteral("rightMargin") && p->kind() == DomProperty::Number) {
            rightMargin = p->elementNumber();
            continue;
        } else if (propertyName == QStringLiteral("bottomMargin") && p->kind() == DomProperty::Number) {
            bottomMargin = p->elementNumber();
            continue;
        } else if (propertyName == QStringLiteral("frameShadow"))
            frameShadowEncountered = true;

        bool stdset = m_stdsetdef;
        if (p->hasAttributeStdset())
            stdset = p->attributeStdset();

        QString setFunction;

        if (stdset) {
            setFunction = QStringLiteral(".")
                + propertyName
                + QStringLiteral(" = ");
        } else {
            setFunction = QStringLiteral(".setProperty(\"")
                + propertyName
                + QStringLiteral("\", Qt::Variant.new(");
        }

        QString varNewName = varName;

        switch (p->kind()) {
        case DomProperty::Bool: {
            propertyValue = p->elementBool();
            break;
        }
        case DomProperty::Color: {
            DomColor *c = p->elementColor();
            propertyValue = QString::fromLatin1("Qt::Color.new(%1, %2, %3)")
                  .arg(c->elementRed())
                  .arg(c->elementGreen())
                  .arg(c->elementBlue()); }
            break;
        case DomProperty::Cstring:
            if (propertyName == QStringLiteral("buddy") && m_uic->customWidgetsInfo()->extends(className, QStringLiteral("QLabel"))) {
                m_buddies.append(Buddy(varName, p->elementCstring()));
            } else {
                if (stdset)
                    propertyValue = fixString(p->elementCstring(), m_option.indent);
                else
                    propertyValue = QStringLiteral("Qt::ByteArray.new(") + fixString(p->elementCstring(), m_option.indent) + QStringLiteral(")");
            }
            break;
        case DomProperty::Cursor:
            propertyValue = QString::fromLatin1("Qt::Cursor.new(%1)")
                            .arg(p->elementCursor());
            break;
        case DomProperty::CursorShape:
            if (p->hasAttributeStdset() && !p->attributeStdset())
                varNewName += QStringLiteral(".viewport()");
            propertyValue = QString::fromLatin1("Qt::Cursor.new(Qt::%1)")
                            .arg(p->elementCursorShape());
            break;
        case DomProperty::Enum:
            propertyValue = p->elementEnum();
            if (!propertyValue.contains(QStringLiteral("::")))
                propertyValue.prepend(className + QStringLiteral("::"));
            propertyValue = m_driver->rubyClassName(propertyValue);
            break;
        case DomProperty::Set:
            propertyValue = p->elementSet();
            if (!propertyValue.contains(QStringLiteral("::")))
                propertyValue.prepend(className + QStringLiteral("::"));
            propertyValue = m_driver->rubyClassName(propertyValue);
            propertyValue.replace("|", "|");
            break;
        case DomProperty::Font:
            propertyValue = writeFontProperties(p->elementFont());
            break;
        case DomProperty::IconSet:
            propertyValue = writeIconProperties(p->elementIconSet());
            break;

        case DomProperty::Pixmap:
            propertyValue = pixCall(p);
            break;

        case DomProperty::Palette: {
            DomPalette *pal = p->elementPalette();
            QString paletteName = QString("@") + m_driver->unique(QStringLiteral("palette"));
            m_output << m_option.indent << paletteName << " = Qt::Palette.new\n";

            writeColorGroup(pal->elementActive(), QStringLiteral("Qt::Palette::Active"), paletteName);
            writeColorGroup(pal->elementInactive(), QStringLiteral("Qt::Palette::Inactive"), paletteName);
            writeColorGroup(pal->elementDisabled(), QStringLiteral("Qt::Palette::Disabled"), paletteName);

            propertyValue = paletteName;
            break;
        }
        case DomProperty::Point: {
            DomPoint *po = p->elementPoint();
            propertyValue = QString::fromLatin1("Qt::Point.new(%1, %2)")
                            .arg(po->elementX()).arg(po->elementY());
            break;
        }
        case DomProperty::PointF: {
            DomPointF *pof = p->elementPointF();
            propertyValue = QString::fromLatin1("Qt::PointF.new(%1, %2)")
                            .arg(pof->elementX()).arg(pof->elementY());
            break;
        }
        case DomProperty::Rect: {
            DomRect *r = p->elementRect();
            propertyValue = QString::fromLatin1("Qt::Rect.new(%1, %2, %3, %4)")
                            .arg(r->elementX()).arg(r->elementY())
                            .arg(r->elementWidth()).arg(r->elementHeight());
            break;
        }
        case DomProperty::RectF: {
            DomRectF *rf = p->elementRectF();
            propertyValue = QString::fromLatin1("Qt::RectF.new(%1, %2, %3, %4)")
                            .arg(rf->elementX()).arg(rf->elementY())
                            .arg(rf->elementWidth()).arg(rf->elementHeight());
            break;
        }
        case DomProperty::Locale: {
             const DomLocale *locale = p->elementLocale();
             propertyValue = QString::fromLatin1("Qt::Locale.new(Qt::Locale::%1, Qt::Locale::%2)")
                             .arg(locale->attributeLanguage()).arg(locale->attributeCountry());
            break;
        }
        case DomProperty::SizePolicy: {
            const QString spName = writeSizePolicy( p->elementSizePolicy());
            m_output << m_option.indent << spName << QString::fromLatin1(
                ".heightForWidth = %1.sizePolicy.hasHeightForWidth\n")
                .arg(varName);

            propertyValue = spName;
            break;
        }
        case DomProperty::Size: {
             DomSize *s = p->elementSize();
              propertyValue = QString::fromLatin1("Qt::Size.new(%1, %2)")
                             .arg(s->elementWidth()).arg(s->elementHeight());
            break;
        }
        case DomProperty::SizeF: {
            DomSizeF *sf = p->elementSizeF();
             propertyValue = QString::fromLatin1("Qt::SizeF.new(%1, %2)")
                            .arg(sf->elementWidth()).arg(sf->elementHeight());
            break;
        }
        case DomProperty::String: {
            if (propertyName == QStringLiteral("objectName")) {
                QString v = p->elementString()->text();
                if (    (varName.startsWith("@") && v == varName.mid(1))
                        || v.mid(0,1).toLower() + v.mid(1) == varName )
                    break;

                // ### qWarning("Deprecated: the property `objectName' is different from the variable name");
            }

            if (p->elementString()->hasAttributeNotr()
                    && toBool(p->elementString()->attributeNotr())) {
                propertyValue = fixString(p->elementString()->text(), m_option.indent);
            } else {
                propertyValue = trCall(p->elementString());
            }
            break;
        }
        case DomProperty::Number:
            propertyValue = QString::number(p->elementNumber());
            break;
        case DomProperty::UInt:
            propertyValue = QString::number(p->elementUInt());
            break;
        case DomProperty::LongLong:
            propertyValue = QString::number(p->elementLongLong());
            break;
        case DomProperty::ULongLong:
            propertyValue += QString::number(p->elementULongLong());
            break;
        case DomProperty::Float:
            propertyValue = QString::number(p->elementFloat());
            break;
        case DomProperty::Double:
            propertyValue = QString::number(p->elementDouble());
            break;
        case DomProperty::Char: {
            DomChar *c = p->elementChar();
            propertyValue = QString::fromLatin1("Qt::Char.new(%1)")
                            .arg(c->elementUnicode());
            break;
        }
        case DomProperty::Date: {
            DomDate *d = p->elementDate();
            propertyValue = QString::fromLatin1("Qt::Date.new(%1, %2, %3)")
                            .arg(d->elementYear())
                            .arg(d->elementMonth())
                            .arg(d->elementDay());
            break;
        }
        case DomProperty::Time: {
            DomTime *t = p->elementTime();
            propertyValue = QString::fromLatin1("Qt::Time.new(%1, %2, %3)")
                            .arg(t->elementHour())
                            .arg(t->elementMinute())
                            .arg(t->elementSecond());
            break;
        }
        case DomProperty::DateTime: {
            DomDateTime *dt = p->elementDateTime();
            propertyValue = QString::fromLatin1("Qt::DateTime.new(Qt::Date.new(%1, %2, %3), Qt::Time.new(%4, %5, %6))")
                            .arg(dt->elementYear())
                            .arg(dt->elementMonth())
                            .arg(dt->elementDay())
                            .arg(dt->elementHour())
                            .arg(dt->elementMinute())
                            .arg(dt->elementSecond());
            break;
        }
        case DomProperty::StringList:
            propertyValue = QStringLiteral("[]");
            if (p->elementStringList()->elementString().size()) {
                QStringList lst = p->elementStringList()->elementString();
                for (int i=0; i<lst.size(); ++i) {
                    propertyValue += QStringLiteral(" << ") + fixString(lst.at(i), m_option.indent);
                }
            }
            break;

        case DomProperty::Url: {
            DomUrl* u = p->elementUrl();
            propertyValue = QString::fromLatin1("Qt::Url.new(%1)")
                            .arg(fixString(u->elementString()->text(), m_option.indent));
            break;
        }
        case DomProperty::Brush:
            propertyValue = writeBrushInitialization(p->elementBrush());
            break;
        case DomProperty::Unknown:
            break;
        }

        if (propertyValue.size()) {
            const bool needsTranslation = p->kind() == DomProperty::String && (!p->elementString()->hasAttributeNotr() || !toBool(p->elementString()->attributeNotr()));
        
            QTextStream &o = needsTranslation ? m_refreshOut : m_output;

            o << m_option.indent << varNewName << setFunction << propertyValue;
            if (!stdset) {
                o << "))";
            }
            o << "\n";
        }
    }
    if (leftMargin != -1 || topMargin != -1 || rightMargin != -1 || bottomMargin != -1) {
        QString objectName = varName;
        if (m_widgetChain.top()) {
            const QString parentWidget = m_widgetChain.top()->attributeClass();

            if (!m_layoutChain.top() && (m_uic->customWidgetsInfo()->extends(parentWidget, QStringLiteral("Q3GroupBox"))
                        || m_uic->customWidgetsInfo()->extends(parentWidget, QStringLiteral("Q3ButtonGroup")))) {
                objectName = m_driver->findOrInsertWidget(m_widgetChain.top()) + QStringLiteral(".layout()");
            }
        }
        m_output << m_option.indent << objectName << QStringLiteral(".setContentsMargins(")
                 << leftMargin << QStringLiteral(", ")
                 << topMargin << QStringLiteral(", ")
                 << rightMargin << QStringLiteral(", ")
                 << bottomMargin << QStringLiteral(")\n");
    }
}

QString  WriteInitialization::writeSizePolicy(const DomSizePolicy *sp)
{

    // check cache
    const SizePolicyHandle sizePolicyHandle(sp);
    const SizePolicyNameMap::const_iterator it = m_sizePolicyNameMap.constFind(sizePolicyHandle);
    if ( it != m_sizePolicyNameMap.constEnd()) {
        return it.value();
    }


    // insert with new name
    const QString spName = QString("@") + m_driver->unique(QStringLiteral("sizePolicy"));
    m_sizePolicyNameMap.insert(sizePolicyHandle, spName);

    m_output << m_option.indent << spName << " = Qt::SizePolicy.new";
    do {
        if (sp->hasElementHSizeType() && sp->hasElementVSizeType()) {
            m_output << "(" << sp->elementHSizeType()
                << ", " << sp->elementVSizeType() << ")\n";
            break;
        }
        if (sp->hasAttributeHSizeType() && sp->hasAttributeVSizeType()) {
                m_output << "(Qt::SizePolicy::" << sp->attributeHSizeType() << ", Qt::SizePolicy::"
                << sp->attributeVSizeType() << ")\n";
            break;
        }
        m_output << "\n";
    } while (false);

    m_output << m_option.indent << spName << ".setHorizontalStretch("
        << sp->elementHorStretch() << ")\n";
    m_output << m_option.indent << spName << ".setVerticalStretch("
        << sp->elementVerStretch() << ")\n";
    return spName;
}
// Check for a font with the given properties in the FontPropertiesNameMap
// or create a new one. Returns the name.

QString WriteInitialization::writeFontProperties(const DomFont *f)
{
    // check cache
    const FontHandle fontHandle(f);
    const FontPropertiesNameMap::const_iterator it = m_fontPropertiesNameMap.constFind(fontHandle);
    if ( it != m_fontPropertiesNameMap.constEnd()) {
        return it.value();
    }

    // insert with new name
    const QString fontName = QString("@") + m_driver->unique(QStringLiteral("font"));
    m_fontPropertiesNameMap.insert(FontHandle(f), fontName);

    m_output << m_option.indent << fontName << " = Qt::Font.new\n";
    if (f->hasElementFamily() && !f->elementFamily().isEmpty()) {
        m_output << m_option.indent << fontName << ".family = " << fixString(f->elementFamily(), m_option.indent)
            << "\n";
    }
    if (f->hasElementPointSize() && f->elementPointSize() > 0) {
         m_output << m_option.indent << fontName << ".pointSize = " << f->elementPointSize()
             << "\n";
    }

    if (f->hasElementBold()) {
        m_output << m_option.indent << fontName << ".bold = "
            << (f->elementBold() ? "true" : "false") << "\n";
    }
    if (f->hasElementItalic()) {
        m_output << m_option.indent << fontName << ".italic = "
            <<  (f->elementItalic() ? "true" : "false") << "\n";
    }
    if (f->hasElementUnderline()) {
        m_output << m_option.indent << fontName << ".underline = "
            << (f->elementUnderline() ? "true" : "false") << "\n";
    }
    if (f->hasElementWeight() && f->elementWeight() > 0) {
        m_output << m_option.indent << fontName << ".weight = "
            << f->elementWeight() << "" << endl;
    }
    if (f->hasElementStrikeOut()) {
         m_output << m_option.indent << fontName << ".strikeOut = "
            << (f->elementStrikeOut() ? "true" : "false") << "\n";
    }
    if (f->hasElementKerning()) {
        m_output << m_option.indent << fontName << ".kerning = "
            << (f->elementKerning() ? "true" : "false") << "\n";
    }
    if (f->hasElementAntialiasing()) {
        m_output << m_option.indent << fontName << ".styleStrategy = "
            << (f->elementAntialiasing() ? "Qt::Font::PreferDefault" : "Qt::Font::NoAntialias") << "\n";
    }
    if (f->hasElementStyleStrategy()) {
         m_output << m_option.indent << fontName << ".styleStrategy = Qt::Font::"
            << f->elementStyleStrategy() << "\n";
    }
    return  fontName;
}

QString WriteInitialization::writeIconProperties(const DomResourceIcon *i)
{
    // check cache
    const IconHandle iconHandle(i);
    const IconPropertiesNameMap::const_iterator it = m_iconPropertiesNameMap.constFind(iconHandle);
    if (it != m_iconPropertiesNameMap.constEnd()) {
        return it.value();
    }

    // insert with new name
    const QString iconName = m_driver->unique(QStringLiteral("icon"));
    m_iconPropertiesNameMap.insert(IconHandle(i), iconName);
    if (isIconFormat44(i)) {
        const QString pixmap = QStringLiteral("Qt::Pixmap.new");
        m_output << m_option.indent << iconName << " = Qt::Icon.new\n";
        if (i->hasElementNormalOff())
            m_output << m_option.indent << iconName << ".addPixmap(" << pixCall(pixmap, i->elementNormalOff()->text()) << ", Qt::Icon::Normal, Qt::Icon::Off)\n";
        if (i->hasElementNormalOn())
            m_output << m_option.indent << iconName << ".addPixmap(" << pixCall(pixmap, i->elementNormalOn()->text()) << ", Qt::Icon::Normal, Qt::Icon::On)\n";
        if (i->hasElementDisabledOff())
            m_output << m_option.indent << iconName << ".addPixmap(" << pixCall(pixmap, i->elementDisabledOff()->text()) << ", Qt::Icon::Disabled, Qt::Icon::Off)\n";
        if (i->hasElementDisabledOn())
            m_output << m_option.indent << iconName << ".addPixmap(" << pixCall(pixmap, i->elementDisabledOn()->text()) << ", Qt::Icon::Disabled, Qt::Icon::On)\n";
        if (i->hasElementActiveOff())
            m_output << m_option.indent << iconName << ".addPixmap(" << pixCall(pixmap, i->elementActiveOff()->text()) << ", Qt::Icon::Active, Qt::Icon::Off)\n";
        if (i->hasElementActiveOn())
            m_output << m_option.indent << iconName << ".addPixmap(" << pixCall(pixmap, i->elementActiveOn()->text()) << ", Qt::Icon::Active, Qt::Icon::On)\n";
        if (i->hasElementSelectedOff())
            m_output << m_option.indent << iconName << ".addPixmap(" << pixCall(pixmap, i->elementSelectedOff()->text()) << ", Qt::Icon::Selected, Qt::Icon::Off)\n";
        if (i->hasElementSelectedOn())
            m_output << m_option.indent << iconName << ".addPixmap(" << pixCall(pixmap, i->elementSelectedOn()->text()) << ", Qt::Icon::Selected, Qt::Icon::On)\n";
    } else { // pre-4.4 legacy
        m_output <<  m_option.indent << iconName << " = " << pixCall(QStringLiteral("Qt::Icon.new"), i->text())<< "\n";
    }
    return iconName;
}

QString WriteInitialization::domColor2QString(const DomColor *c)
{
    if (c->hasAttributeAlpha())
        return QString::fromLatin1("Qt::Color.new(%1, %2, %3, %4)")
            .arg(c->elementRed())
            .arg(c->elementGreen())
            .arg(c->elementBlue())
            .arg(c->attributeAlpha());
    return QString::fromLatin1("Qt::Color.new(%1, %2, %3)")
        .arg(c->elementRed())
        .arg(c->elementGreen())
        .arg(c->elementBlue());
}

void WriteInitialization::writeColorGroup(DomColorGroup *colorGroup, const QString &group, const QString &paletteName)
{
    if (!colorGroup)
        return;

    // old format
    QList<DomColor*> colors = colorGroup->elementColor();
    for (int i=0; i<colors.size(); ++i) {
        DomColor *color = colors.at(i);

        m_output << m_option.indent << paletteName << ".setColor(" << group
            << ", " << i 
            << ", " << domColor2QString(color)
            << ")\n";
    }

    // new format
    const QList<DomColorRole *> colorRoles = colorGroup->elementColorRole();
    QListIterator<DomColorRole *> itRole(colorRoles);
    while (itRole.hasNext()) {
        const DomColorRole *colorRole = itRole.next();
        if (colorRole->hasAttributeRole()) {
            const QString brushName = writeBrushInitialization(colorRole->elementBrush());
            m_output << m_option.indent << paletteName << ".setBrush(" << group
                << ", " << "Qt::Palette::" << colorRole->attributeRole()
                << ", " << brushName << ")\n";
        }
    }
}
// Write initialization for brush unless it is found in the cache. Returns the name to use
// in an expression.
QString WriteInitialization::writeBrushInitialization(const DomBrush *brush)
{
    // Simple solid, colored  brushes are cached
    const bool solidColoredBrush = !brush->hasAttributeBrushStyle() || brush->attributeBrushStyle() == QStringLiteral("SolidPattern");
    uint rgb = 0;
    if (solidColoredBrush) {
        if (const DomColor *color = brush->elementColor()) {
            rgb = ((color->elementRed() & 0xFF) << 24) |
                  ((color->elementGreen() & 0xFF) << 16) |
                  ((color->elementBlue() & 0xFF) << 8) |
                  ((color->attributeAlpha() & 0xFF));
            const ColorBrushHash::const_iterator cit = m_colorBrushHash.constFind(rgb);
            if (cit != m_colorBrushHash.constEnd())
                return cit.value();
        }
    }
    // Create and enter into cache if simple
    const QString brushName = m_driver->unique(QStringLiteral("brush"));
    writeBrush(brush, brushName);
    if (solidColoredBrush)
        m_colorBrushHash.insert(rgb, brushName);
    return brushName;
}

void WriteInitialization::writeBrush(const DomBrush *brush, const QString &brushName)
{
    QString style = QStringLiteral("SolidPattern");
    if (brush->hasAttributeBrushStyle())
        style = brush->attributeBrushStyle();

    if (style == QStringLiteral("LinearGradientPattern") ||
            style == QStringLiteral("RadialGradientPattern") ||
            style == QStringLiteral("ConicalGradientPattern")) {
        DomGradient *gradient = brush->elementGradient();
        QString gradientType = gradient->attributeType();
        QString gradientName = m_driver->unique(QStringLiteral("gradient"));
        if (gradientType == QStringLiteral("LinearGradient")) {
            m_output << m_option.indent << gradientName << " = Qt::LinearGradient.new"
                << "(" << gradient->attributeStartX()
                << ", " << gradient->attributeStartY()
                << ", " << gradient->attributeEndX()
                << ", " << gradient->attributeEndY() << ")\n";
        } else if (gradientType == QStringLiteral("RadialGradient")) {
            m_output << m_option.indent << gradientName << " = Qt::RadialGradient.new"
                << "(" << gradient->attributeCentralX()
                << ", " << gradient->attributeCentralY()
                << ", " << gradient->attributeRadius()
                << ", " << gradient->attributeFocalX()
                << ", " << gradient->attributeFocalY() << ")\n";
        } else if (gradientType == QStringLiteral("ConicalGradient")) {
            m_output << m_option.indent << gradientName << " = Qt::ConicalGradient.new"
                << "(" << gradient->attributeCentralX()
                << ", " << gradient->attributeCentralY()
                << ", " << gradient->attributeAngle() << ")\n";
        }

        m_output << m_option.indent << gradientName << ".spread = Qt::Gradient::"
            << gradient->attributeSpread() << "\n";

        if (gradient->hasAttributeCoordinateMode()) {
            m_output << m_option.indent << gradientName << ".coordinateMode = Qt::Gradient::"
                << gradient->attributeCoordinateMode() << "\n";
        }

        const QList<DomGradientStop *> stops = gradient->elementGradientStop();
        QListIterator<DomGradientStop *> it(stops);
        while (it.hasNext()) {
            const DomGradientStop *stop = it.next();
            const DomColor *color = stop->elementColor();
            m_output << m_option.indent << gradientName << ".setColorAt("
                << stop->attributePosition() << ", "
                << domColor2QString(color) << ")\n";
        }
        m_output << m_option.indent << brushName << " = Qt::Brush.new" << "("
            << gradientName << ")\n";
    } else if (style == QStringLiteral("TexturePattern")) {
        DomProperty *property = brush->elementTexture();

        m_output << m_option.indent << brushName << " = Qt::Brush.new" <<  "("
            << pixCall(property) << ")\n";
    } else {
        DomColor *color = brush->elementColor();
        m_output << m_option.indent << brushName << " = Qt::Brush.new" <<  "("
            << domColor2QString(color) << ")\n";

        m_output << m_option.indent << brushName << ".style = "
            << "Qt::" << style << "\n";
    }
}

void WriteInitialization::acceptCustomWidget(DomCustomWidget *node)
{
    Q_UNUSED(node);
}

void WriteInitialization::acceptCustomWidgets(DomCustomWidgets *node)
{
    Q_UNUSED(node);
}

void WriteInitialization::acceptTabStops(DomTabStops *tabStops)
{
    QString lastName;

    const QStringList l = tabStops->elementTabStop();
    for (int i=0; i<l.size(); ++i) {
        QString name = l.at(i);

        if (!m_registeredWidgets.contains(name)) {
            fprintf(stderr, "'%s' isn't a valid widget\n", name.toLocal8Bit().constData());
            continue;
        }

        if (i == 0) {
            lastName = name;
            continue;
        } else if (name.isEmpty() || lastName.isEmpty()) {
            continue;
        }

        m_output << m_option.indent << "Qt::Widget.setTabOrder(@" << lastName << ", @" << name << ")\n";

        lastName = name;
    }
}

void WriteInitialization::initializeQ3ListBox(DomWidget *w)
{
    QString varName = toRubyIdentifier(m_driver->findOrInsertWidget(w));
    QString className = w->attributeClass();

    QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    m_refreshOut << m_option.indent << varName << ".clear()\n";

    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        DomProperty *text = properties.value(QStringLiteral("text"));
        DomProperty *pixmap = properties.value(QStringLiteral("pixmap"));
        if (!(text || pixmap))
            continue;

        m_refreshOut << m_option.indent << varName << ".insertItem(";
        if (pixmap) {
            m_refreshOut << pixCall(pixmap);

            if (text)
                m_refreshOut << ", ";
        }
        if (text)
            m_refreshOut << trCall(text->elementString());
        m_refreshOut << ")\n";
    }
}

void WriteInitialization::initializeQ3IconView(DomWidget *w)
{
    QString varName = toRubyIdentifier(m_driver->findOrInsertWidget(w));
    QString className = w->attributeClass();

    QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    m_refreshOut << m_option.indent << varName << ".clear()\n";

    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        DomProperty *text = properties.value(QStringLiteral("text"));
        DomProperty *pixmap = properties.value(QStringLiteral("pixmap"));
        if (!(text || pixmap))
            continue;

        QString itemName = m_driver->unique(QStringLiteral("__item"));
        m_refreshOut << "\n";
        m_refreshOut << m_option.indent << itemName << " = Qt::3IconViewItem.new(" << varName << ")\n";

        if (pixmap) {
            m_refreshOut << m_option.indent << itemName << ".pixmap = " << pixCall(pixmap) << "\n";
        }

        if (text) {
            m_refreshOut << m_option.indent << itemName << ".text = " << trCall(text->elementString()) << "\n";
        }
    }
}

void WriteInitialization::initializeQ3ListView(DomWidget *w)
{
    const QString varName = toRubyIdentifier(m_driver->findOrInsertWidget(w));
    const QString className = w->attributeClass();

    // columns
    QList<DomColumn*> columns = w->elementColumn();
    for (int i=0; i<columns.size(); ++i) {
        DomColumn *column = columns.at(i);

        const DomPropertyMap properties = propertyMap(column->elementProperty());
        const DomProperty *text = properties.value(QStringLiteral("text"));
        const DomProperty *pixmap = properties.value(QStringLiteral("pixmap"));
        const DomProperty *clickable = properties.value(QStringLiteral("clickable"));
        const DomProperty *resizable = properties.value(QStringLiteral("resizable"));

        QString txt = trCall(text->elementString());
        m_output << m_option.indent << varName << ".addColumn(" << txt << ")\n";
        m_refreshOut << m_option.indent << varName << ".header.setLabel(" << i << ", " << txt << ")\n";

        if (pixmap) {
            m_output << m_option.indent << varName << ".header.setLabel("
                   << varName << ".header.count - 1, " << pixCall(pixmap) << ", " << txt << ")\n";
        }

        if (clickable != 0) {
            m_output << m_option.indent << varName << ".header.setClickEnabled(" << clickable->elementBool() << ", " << varName << ".header().count() - 1)\n";
        }

        if (resizable != 0) {
            m_output << m_option.indent << varName << ".header.setResizeEnabled(" << resizable->elementBool() << ", " << varName << ".header().count() - 1)\n";
        }
    }

    if (w->elementItem().size()) {
        m_refreshOut << m_option.indent << varName << ".clear\n";

        initializeQ3ListViewItems(className, varName, w->elementItem());
    }
}

void WriteInitialization::initializeQ3ListViewItems(const QString &className, const QString &varName, const QList<DomItem *> &items)
{
    if (items.isEmpty())
        return;

    // items
    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QString itemName = m_driver->unique(QStringLiteral("__item"));
        m_refreshOut << "\n";
        m_refreshOut << m_option.indent << itemName << " = new Qt3::ListViewItem.new(" << varName << ")\n";

        int textCount = 0, pixCount = 0;
        const DomPropertyList properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            const DomProperty *p = properties.at(i);
            if (p->attributeName() == QStringLiteral("text"))
                m_refreshOut << m_option.indent << itemName << ".setText(" << textCount++ << ", "
                           << trCall(p->elementString()) << ")\n";

            if (p->attributeName() == QStringLiteral("pixmap"))
                m_refreshOut << m_option.indent << itemName << ".setPixmap(" << pixCount++ << ", "
                           << pixCall(p) << ")\n";
        }

        if (item->elementItem().size()) {
            m_refreshOut << m_option.indent << itemName << ".open = true\n";
            initializeQ3ListViewItems(className, itemName, item->elementItem());
        }
    }
}

void WriteInitialization::initializeQ3Table(DomWidget *w)
{
    const QString varName = m_driver->findOrInsertWidget(w);
    const QString className = w->attributeClass();

    // columns
    const QList<DomColumn*> columns = w->elementColumn();

    for (int i=0; i<columns.size(); ++i) {
        const DomColumn *column = columns.at(i);

        const DomPropertyMap properties = propertyMap(column->elementProperty());
        const DomProperty *text = properties.value(QStringLiteral("text"));
        const DomProperty *pixmap = properties.value(QStringLiteral("pixmap"));

        m_refreshOut << m_option.indent << varName << ".horizontalHeader.setLabel(" << i << ", ";
        if (pixmap) {
            m_refreshOut << pixCall(pixmap) << ", ";
        }
        m_refreshOut << trCall(text->elementString()) << ")\n";
    }

    // rows
    const QList<DomRow*> rows = w->elementRow();
    for (int i=0; i<rows.size(); ++i) {
        const DomRow *row = rows.at(i);

        const DomPropertyMap properties = propertyMap(row->elementProperty());
        const DomProperty *text = properties.value(QStringLiteral("text"));
        const DomProperty *pixmap = properties.value(QStringLiteral("pixmap"));

        m_refreshOut << m_option.indent << varName << ".verticalHeader.setLabel(" << i << ", ";
        if (pixmap) {
            m_refreshOut << pixCall(pixmap) << ", ";
        }
        m_refreshOut << trCall(text->elementString()) << ")\n";
    }


    //initializeQ3TableItems(className, varName, w->elementItem());
}

void WriteInitialization::initializeQ3TableItems(const QString &className, const QString &varName, const QList<DomItem *> &items)
{
    Q_UNUSED(className);
    Q_UNUSED(varName);
    Q_UNUSED(items);
}

QString WriteInitialization::iconCall(const DomProperty *icon)
{
    if (icon->kind() == DomProperty::IconSet)
        return writeIconProperties(icon->elementIconSet());
    return pixCall(icon);
}

QString WriteInitialization::pixCall(const DomProperty *p) const
{
    QString type, s;
    switch (p->kind()) {
    case DomProperty::IconSet:
        type = QStringLiteral("Qt::Icon.new");
        s = p->elementIconSet()->text();
        break;
    case DomProperty::Pixmap:
        type = QStringLiteral("Qt::Pixmap.new");
        s = p->elementPixmap()->text();
        break;
    default:
        qWarning() << "Warning: Unknown icon format encountered. The ui-file was generated with a too-recent version of Designer.";
        return QStringLiteral("QIcon()");
        break;
    }
    return pixCall(type, s);
}

QString WriteInitialization::pixCall(const QString &t, const QString &text) const
{
    QString type = t;
    if (text.isEmpty()) {
        type += QStringLiteral("()");
        return type;
    }
    if (const DomImage *image = findImage(text)) {

        if (m_option.extractImages) {
            const QString format = image->elementData()->attributeFormat();
            const QString extension = format.left(format.indexOf(QLatin1Char('.'))).toLower();
            QString rc = QStringLiteral("Qt::Pixmap.new(\":/");
            rc += m_generatedClass;
            rc += QStringLiteral("/images/");
            rc += text;
            rc += QLatin1Char('.');
            rc += extension;
            rc += QStringLiteral("\")");
            return rc;
        }

        QString rc = WriteIconInitialization::iconFromDataFunction();
        rc += QLatin1Char('(');
        rc += text;
        rc += QStringLiteral("_ID)");
        return rc;
    }

    QString pixFunc = m_uic->pixmapFunction();
    type += pixFunc;
    type += QLatin1Char('(');
    type += fixString(text, m_option.indent);
    type += QStringLiteral(")");
    return type;
}

void WriteInitialization::initializeComboBox(DomWidget *w)
{
    QString varName = toRubyIdentifier(m_driver->findOrInsertWidget(w));
    const QString className = w->attributeClass();

    const QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    // If possible use qcombobox's addItems() which is much faster than a bunch of addItem() calls
    bool noIcons = true;
    QStringList list;
    for (int i=0; i<items.size(); ++i) {
        const DomItem *item = items.at(i);
        const DomPropertyMap properties = propertyMap(item->elementProperty());
        const DomProperty *text = properties.value(QStringLiteral("text"));
        const DomProperty *pixmap = properties.value(QStringLiteral("icon"));
        if (pixmap != 0) {
            noIcons = false;
            break;
        }
        list.append(trCall(text->elementString()));
    }

    if (noIcons) {
        m_refreshOut << m_option.indent << varName << ".insertItems(0, [" ;
        for (int i = 0; i < list.size(); ++i) {
            m_refreshOut << list.at(i);
            if (i != (list.size() - 1)) {
                m_refreshOut << ",\n" << m_option.indent << m_option.indent;
            }
        }
        m_refreshOut << "])\n";
    } else {
        for (int i = 0; i < items.size(); ++i) {
            const DomItem *item = items.at(i);
            const DomPropertyMap properties = propertyMap(item->elementProperty());
            const DomProperty *text = properties.value(QStringLiteral("text"));
            const DomProperty *icon = properties.value(QStringLiteral("icon"));

            QString iconValue;
            if (icon)
                iconValue = iconCall(icon);

            m_output << m_option.indent << varName << ".addItem(";
            if (icon)
                m_output << iconValue << ", ";
            m_output << "\"\")\n";

            if (!toString(text->elementString()).isEmpty())
                m_refreshOut << m_option.indent << varName << ".setItemText(" << i << ", " << trCall(text->elementString()) << ")\n";
        }
        m_refreshOut << "\n";
    }
}

QString WriteInitialization::disableSorting(DomWidget *w, const QString &varName)
{
    // turn off sortingEnabled to force programmatic item order (setItem())
    QString tempName;
    if (!w->elementItem().isEmpty()) {
        tempName = m_driver->unique(QStringLiteral("__sortingEnabled"));
        m_refreshOut << "\n";
        m_refreshOut << m_option.indent << tempName
            << " = " << varName << ".sortingEnabled?\n";
        m_refreshOut << m_option.indent << varName << ".sortingEnabled = false\n";
    }
    return tempName;
}

void WriteInitialization::enableSorting(DomWidget *w, const QString &varName, const QString &tempName)
{
    if (!w->elementItem().isEmpty()) {
        m_refreshOut << "\n";
        m_refreshOut << m_option.indent << varName << ".sortingEnabled = " << tempName << "\n";
    }
}

void WriteInitialization::initializeListWidget(DomWidget *w)
{
    QString varName = toRubyIdentifier(m_driver->findOrInsertWidget(w));
    const QString className = w->attributeClass();

    const QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    QString tempName = disableSorting(w, varName);
    // items
    for (int i = 0; i < items.size(); ++i) {
        const DomItem *item = items.at(i);

        const DomPropertyMap properties = propertyMap(item->elementProperty());
        const DomProperty *text = properties.value(QStringLiteral("text"));
        const DomProperty *icon = properties.value(QStringLiteral("icon"));

        QString itemCreation = QStringLiteral("Qt::ListWidgetItem.new(") + varName + QStringLiteral(")\n");
        if (icon) {
            const QString iconValue = iconCall(icon);
            const QString itemName = m_driver->unique(QStringLiteral("__listItem"));
            m_output << m_option.indent << itemName << " = " << itemCreation;
            m_output << m_option.indent << itemName << ".icon = " << iconValue << "\n";
        } else {
            m_output << m_option.indent << itemCreation;
        }

        if (!toString(text->elementString()).isEmpty()) {
            m_refreshOut << m_option.indent << varName << ".item(" << i << ").text = " << trCall(text->elementString()) << "\n";
        }
    }
    enableSorting(w, varName, tempName);
}

void WriteInitialization::initializeTreeWidget(DomWidget *w)
{
    QString varName = toRubyIdentifier(m_driver->findOrInsertWidget(w));
    const QString className = w->attributeClass();

    // columns
    const QList<DomColumn*> columns = w->elementColumn();
    for (int i = 0; i < columns.size(); ++i) {
        const DomColumn *column = columns.at(i);

        const DomPropertyMap properties = propertyMap(column->elementProperty());
        const DomProperty *text = properties.value(QStringLiteral("text"));
        const DomProperty *icon = properties.value(QStringLiteral("icon"));

        if (!toString(text->elementString()).isEmpty()) {
            const QString txt = trCall(text->elementString());
            m_refreshOut << m_option.indent << varName << ".headerItem.setText(" << i << ", " << txt << ")\n";
        }

        if (icon) {
            const QString iconValue = iconCall(icon);
            m_output << m_option.indent << varName << ".headerItem.setIcon("
                   << i << ", " << iconValue << ")\n";
        }
    }

    if (w->elementItem().size()) {
        QString tempName = disableSorting(w, varName);

        initializeTreeWidgetItems(className, varName, w->elementItem(), varName + QStringLiteral(".topLevelItem("));

        enableSorting(w, varName, tempName);
    }
}

void WriteInitialization::initializeTreeWidgetItems(const QString &className, const QString &varName, const QList<DomItem *> &items, const QString &parentPath)
{
    if (items.isEmpty())
        return;

    // items
    for (int i = 0; i < items.size(); ++i) {
        const DomItem *item = items.at(i);

        int textCount = 0;
        const DomPropertyList properties = item->elementProperty();
        QStringList icons;
        for (int j = 0; j < properties.size(); ++j) {
            const DomProperty *p = properties.at(j);
            if (p->attributeName() == QStringLiteral("text")) {
                if (!toString(p->elementString()).isEmpty()) {
                    m_refreshOut << m_option.indent << parentPath << i << ").setText(" << textCount << ", "
                        << trCall(p->elementString()) << ")\n";
                }
                textCount++;
            }

            if (p->attributeName() == QStringLiteral("icon")) {
                const QString iconValue = iconCall(p);
                icons << QStringLiteral(".setIcon(")
                       + QString::number(textCount - 1)
                       + QStringLiteral(", ") + iconValue + QStringLiteral(")\n");
            }
        }
        if (icons.isEmpty() && (item->elementItem().size() == 0)) {
            m_output << m_option.indent << "Qt::TreeWidgetItem.new(" << varName << ")\n";
        } else {
            const QString itemName = m_driver->unique(QStringLiteral("__treeItem"));
            m_output << m_option.indent << itemName << " = Qt::TreeWidgetItem.new(" << varName << ")\n";

            QStringListIterator it(icons);
            while (it.hasNext())
                m_output << m_option.indent << itemName << it.next();

            if (item->elementItem().size()) {
                const QString childPath = parentPath + QString::number(i) + QStringLiteral(").child(");
                initializeTreeWidgetItems(className, itemName, item->elementItem(), childPath);
            }
        }

    }
    m_output << "\n";
}

void WriteInitialization::initializeTableWidget(DomWidget *w)
{
    QString varName = toRubyIdentifier(m_driver->findOrInsertWidget(w));
    QString className = w->attributeClass();

    // columns
    QList<DomColumn *> columns = w->elementColumn();

    if (columns.size() != 0) {
        m_refreshOut << m_option.indent << "if " << varName << ".columnCount < " << columns.size() << "\n"
            << m_option.indent << m_option.indent << varName << ".columnCount = " << columns.size() << "\n"
            << m_option.indent << "end\n";
    }

    for (int i = 0; i < columns.size(); ++i) {
        DomColumn *column = columns.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        DomProperty *text = properties.value(QStringLiteral("text"));
        DomProperty *icon = properties.value(QStringLiteral("icon"));
        if (text || icon) {
            QString itemName = m_driver->unique(QStringLiteral("__colItem"));
            m_refreshOut << "\n";
            m_refreshOut << m_option.indent 
                           << itemName << " = Qt::TableWidgetItem.new\n";

            if (text && text->attributeName() == QStringLiteral("text"))
                m_refreshOut << m_option.indent << itemName << ".setText("
                           << trCall(text->elementString()) << ")\n";

            if (icon && icon->attributeName() == QStringLiteral("icon"))
                m_refreshOut << m_option.indent << itemName << ".setIcon("
                           << pixCall(icon) << ")\n";
            m_refreshOut << m_option.indent << varName << ".setHorizontalHeaderItem("
                           << i << ", " << itemName << ")\n";
        }
    }

    // rows
    QList<DomRow *> rows = w->elementRow();

    if (rows.size() != 0) {
        m_refreshOut << m_option.indent << "if " << varName << ".rowCount < " << rows.size() << "\n"
            << m_option.indent << m_option.indent << varName << ".rowCount = " << rows.size() << "\n"
            << m_option.indent << "end\n";
    }

    for (int i = 0; i < rows.size(); ++i) {
        DomRow *row = rows.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(row->elementProperty());
        DomProperty *text = properties.value(QStringLiteral("text"));
        DomProperty *icon = properties.value(QStringLiteral("icon"));
        if (text || icon) {
            QString itemName = m_driver->unique(QStringLiteral("__rowItem"));
            m_refreshOut << "\n";
            m_refreshOut << m_option.indent
                           << itemName << " = Qt::TableWidgetItem.new\n";

            if (text && text->attributeName() == QStringLiteral("text"))
                m_refreshOut << m_option.indent << itemName << ".setText("
                           << trCall(text->elementString()) << ")\n";

            if (icon && icon->attributeName() == QStringLiteral("icon"))
                m_refreshOut << m_option.indent << itemName << ".setIcon("
                           << pixCall(icon) << ")\n";
            m_refreshOut << m_option.indent << varName << ".setVerticalHeaderItem("
                           << i << ", " << itemName << ")\n";
        }
    }

    // items
    QList<DomItem *> items = w->elementItem();
    for (int i = 0; i < items.size(); ++i) {
        DomItem *item = items.at(i);
        if (item->hasAttributeRow() && item->hasAttributeColumn()) {
            QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
            DomProperty *text = properties.value(QStringLiteral("text"));
            DomProperty *icon = properties.value(QStringLiteral("icon"));
            if (text || icon) {
                QString itemName = m_driver->unique(QStringLiteral("__item"));
                m_refreshOut << "\n";
                m_refreshOut << m_option.indent
                    << itemName << " = Qt::TableWidgetItem.new\n";

                if (text && text->attributeName() == QStringLiteral("text"))
                    m_refreshOut << m_option.indent << itemName << ".setText("
                        << trCall(text->elementString()) << ")\n";

                if (icon && icon->attributeName() == QStringLiteral("icon"))
                    m_refreshOut << m_option.indent << itemName << ".setIcon("
                        << pixCall(icon) << ")\n";
                m_refreshOut << m_option.indent << varName << ".setItem("
                    << item->attributeRow() << ", "
                    << item->attributeColumn() << ", "
                    << itemName << ")\n";
            }
        }
    }
}

QString WriteInitialization::trCall(const QString &str, const QString &commentHint) const
{
    if (str.isEmpty())
        return QStringLiteral("''");

    QString result;
    QString comment = commentHint.isEmpty() ? QString::fromUtf8("nil") : fixString(commentHint, m_option.indent);

    if (m_option.translateFunction.isEmpty()) {
        result = QStringLiteral("Qt::Application.translate(\"");
        result += m_generatedClass;
        result += QStringLiteral("\"");
        result += QStringLiteral(", ");
    } else {
        result = m_option.translateFunction + QStringLiteral("(");
    }

    result += fixString(str, m_option.indent);
    result += QStringLiteral(", ");
    result += comment;

    result += QStringLiteral(")");
    return result;
}

void WriteInitialization::initializeQ3SqlDataTable(DomWidget *w)
{
    QHash<QString, DomProperty*> properties = propertyMap(w->elementProperty());

    DomProperty *frameworkCode = properties.value(QStringLiteral("frameworkCode"), 0);
    if (frameworkCode && toBool(frameworkCode->elementBool()) == false)
        return;

    QString connection;
    QString table;
    QString field;

    DomProperty *db = properties.value(QStringLiteral("database"), 0);
    if (db && db->elementStringList()) {
        QStringList info = db->elementStringList()->elementString();
        connection = info.size() > 0 ? info.at(0) : QString();
        table = info.size() > 1 ? info.at(1) : QString();
        field = info.size() > 2 ? info.at(2) : QString();
    }

    if (table.isEmpty() || connection.isEmpty()) {
        fprintf(stderr, "invalid database connection\n");
        return;
    }

    QString varName = toRubyIdentifier(m_driver->findOrInsertWidget(w));

    m_output << m_option.indent << "if !" << varName << ".sqlCursor.nil?\n";

    m_output << m_option.indent << m_option.indent << varName << ".setSqlCursor(";

    if (connection == QStringLiteral("(default)")) {
        m_output << "Qt3::SqlCursor.new(" << fixString(table, m_option.indent) << "), false, true)\n";
    } else {
        m_output << "Qt3::SqlCursor.new(" << fixString(table, m_option.indent) << ", true, " << connection << "Connection" << "), false, true)\n";
    }
    m_output << m_option.indent << m_option.indent  << varName << ".refresh(Qt3::DataTable::RefreshAll)\n";
    m_output << m_option.indent << "end\n";
}

void WriteInitialization::initializeQ3SqlDataBrowser(DomWidget *w)
{
    QHash<QString, DomProperty*> properties = propertyMap(w->elementProperty());

    DomProperty *frameworkCode = properties.value(QStringLiteral("frameworkCode"), 0);
    if (frameworkCode && toBool(frameworkCode->elementBool()) == false)
        return;

    QString connection;
    QString table;
    QString field;

    DomProperty *db = properties.value(QStringLiteral("database"), 0);
    if (db && db->elementStringList()) {
        QStringList info = db->elementStringList()->elementString();
        connection = info.size() > 0 ? info.at(0) : QString();
        table = info.size() > 1 ? info.at(1) : QString();
        field = info.size() > 2 ? info.at(2) : QString();
    }

    if (table.isEmpty() || connection.isEmpty()) {
        fprintf(stderr, "invalid database connection\n");
        return;
    }

    QString varName = toRubyIdentifier(m_driver->findOrInsertWidget(w));

    m_output << m_option.indent << "if !" << varName << ".sqlCursor.nil?\n";

    m_output << m_option.indent << m_option.indent << varName << ".setSqlCursor(";

    if (connection == QStringLiteral("(default)")) {
        m_output << "Qt::SqlCursor.new(" << fixString(table, m_option.indent) << "), true)\n";
    } else {
        m_output << "Qt::SqlCursor.new(" << fixString(table, m_option.indent) << ", true, " << connection << "Connection" << "), false, true)\n";
    }
    m_output << m_option.indent << m_option.indent << varName << ".refresh\n";
    m_output << m_option.indent << "end\n";
}

void WriteInitialization::initializeMenu(DomWidget *w, const QString &/*parentWidget*/)
{
    const QString menuName = m_driver->findOrInsertWidget(w);
    const QString menuAction = menuName + QStringLiteral("Action");

    const DomAction *action = m_driver->actionByName(menuAction);
    if (action && action->hasAttributeMenu()) {
        m_output << m_option.indent << menuAction << " = " << menuName << ".menuAction\n";
    }
}

QString WriteInitialization::trCall(DomString *str) const
{
    return trCall(toString(str), str->attributeComment());
}

bool WriteInitialization::isValidObject(const QString &name) const
{
    return m_registeredWidgets.contains(name)
        || m_registeredActions.contains(name);
}

QString WriteInitialization::findDeclaration(const QString &name)
{
    QString normalized = Driver::normalizedName(name);

    if (DomWidget *widget = m_driver->widgetByName(normalized))
        return m_driver->findOrInsertWidget(widget);
    else if (DomAction *action = m_driver->actionByName(normalized))
        return m_driver->findOrInsertAction(action);

    return QString();
}

void WriteInitialization::acceptConnection(DomConnection *connection)
{
    QString sender = findDeclaration(connection->elementSender());
    sender = sender.mid(0, 1).toLower() + sender.mid(1);

    if (sender != m_mainWidget) {
        sender = toRubyIdentifier(sender);
    }

    QString receiver = findDeclaration(connection->elementReceiver());
    receiver = receiver.mid(0, 1).toLower() + receiver.mid(1);
    if (receiver != m_mainWidget) {
        receiver = toRubyIdentifier(receiver);
    }

    if (sender.isEmpty() || receiver.isEmpty())
        return;

    m_output << m_option.indent << "Qt::Object.connect("
        << sender
        << ", "
        << "SIGNAL('" << connection->elementSignal() << "')"
        << ", "
        << receiver
        << ", "
        << "SLOT('" << connection->elementSlot() << "')"
        << ")\n";
}

DomImage *WriteInitialization::findImage(const QString &name) const
{
    return m_registeredImages.value(name);
}

DomWidget *WriteInitialization::findWidget(const QString &widgetClass)
{
    for (int i = m_widgetChain.count() - 1; i >= 0; --i) {
        DomWidget *widget = m_widgetChain.at(i);

        if (widget && m_uic->customWidgetsInfo()->extends(widget->attributeClass(), widgetClass))
            return widget;
    }

    return 0;
}

void WriteInitialization::acceptImage(DomImage *image)
{
    if (!image->hasAttributeName())
        return;

    m_registeredImages.insert(image->attributeName(), image);
}

} // namespace Ruby

#if defined(QT_END_NAMESPACE)
  QT_END_NAMESPACE
#endif
