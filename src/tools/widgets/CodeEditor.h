/*
Copyright 2016 Codependable, LLC and Jonathan David Guerin

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma once

#include <QPlainTextEdit>
#include <QObject>
#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>



//modified from the QT code editor example code
//http://qt-project.org/doc/qt-4.8/widgets-codeeditor.html

class QTextDocument;
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

namespace terbit
{

class LineNumberArea;


class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *e);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

private:
    void IndentLines(bool backwards);

    QWidget *m_lineNumberArea;
    QPair<int, int> m_countCache;
    int m_tabSpaces;
};


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor) {
        m_codeEditor = editor;
    }

    QSize sizeHint() const {
        return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        m_codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *m_codeEditor;
};


class JavaScriptHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    JavaScriptHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

}
