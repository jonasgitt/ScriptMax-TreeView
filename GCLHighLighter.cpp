//used for formatting of Script itself

#include "GCLHighLighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    operatorFormat.setForeground(Qt::red);
    operatorFormat.setFontWeight(QFont::Bold);
    parQualFormat.setForeground(Qt::magenta);

    keywordFormat2.setForeground(Qt::cyan);
    QStringList keywordPatterns;
    QStringList keywordPatterns2;
    QStringList operatorPatterns;
    QStringList parQualPatterns;

    keywordPatterns << "\\bPROCEDURE\\b" << "\\ENDPROCEDURE\\b" << "\\bIF\\b"
                        << "\\bENDIF\\b" << "\\bCASE\\b" << "\\bENDCASE\\b"
                        << "\\bLOOP\\b" << "\\bENDLOOP\\b";

    keywordPatterns2 << "\\bELSE\\b" << "\\bELSEIF\\b" << "\\bOTHERWISE\\b"
                        << "\\bEXITIF\\b" << "\\bIS\\b" << "\\bFROM\\b"
                        << "\\bTO\\b" << "\\bSTEP\\b" << "\\bOR\\b" << "\\bAND\\b"
                        << "\\bNOT\\b" << "\\bFORWARD\\b" << "\\bINCLUDE\\b" << "\\bRETURN\\b";

    operatorPatterns << "@";

    parQualPatterns << "PARAMTERES" << "QUALIFIERS" << "RESULT" << "GLOBAL" << "LOCAL";

/*    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                    << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b";

*/
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    foreach (const QString &pattern, keywordPatterns2) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat2;
        highlightingRules.append(rule);
    }

    foreach (const QString &pattern, operatorPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = operatorFormat;
        highlightingRules.append(rule);
    }

    foreach (const QString &pattern, parQualPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = parQualFormat;
        highlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(QColor(64,128,128,255));
    singleLineCommentFormat.setFontWeight(QFont::Bold);
    singleLineCommentFormat.setFontItalic(true);
    rule.pattern = QRegExp("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::red);


    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");

}


void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}

