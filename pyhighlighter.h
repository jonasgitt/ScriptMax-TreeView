#ifndef PYHIGHLIGHTER_H
#define PYHIGHLIGHTER_H

#include <QSyntaxHighlighter>

//! Container to describe a highlighting rule. Based on a regular expression, a relevant match # and the format.
class HighlightingRule
{
public:
 HighlightingRule(const QString &patternStr, int n, const QTextCharFormat &matchingFormat)
 {
      originalRuleStr = patternStr;
      pattern = QRegExp(patternStr);
      nth = n;
      format = matchingFormat;
     }
     QString originalRuleStr;
     QRegExp pattern;
     int nth;
     QTextCharFormat format;
};

//! Implementation of highlighting for Python code.
class KickPythonSyntaxHighlighter : public QSyntaxHighlighter
{
     Q_OBJECT

public:
     KickPythonSyntaxHighlighter(QTextDocument *parent = 0);

protected:
         void highlightBlock(const QString &text);

private:
     QStringList keywords;
     QStringList operators;
     QStringList braces;

     QHash<QString, QTextCharFormat> basicStyles;

     void initializeRules();

     //! Highlighst multi-line strings, returns true if after processing we are still within the multi-line section.
     bool matchMultiline(const QString &text, const QRegExp &delimiter, const int inState, const QTextCharFormat &style);
     const QTextCharFormat getTextCharFormat(const QString &colorName, const QString &style = QString());

     QList<HighlightingRule> rules;
     QRegExp triSingleQuote;
     QRegExp triDoubleQuote;
};

#endif // PYHIGHLIGHTER_H
