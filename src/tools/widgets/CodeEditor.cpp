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
#include "CodeEditor.h"
#include <QTextBlock>
#include <QPainter>

namespace terbit
{

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent), m_tabSpaces(3)
{
   m_countCache.first = -1;
   m_countCache.second = -1;
   m_lineNumberArea = new LineNumberArea(this);

   connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
   connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
   connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

   updateLineNumberAreaWidth(0);
   highlightCurrentLine();

   auto highlighter = new JavaScriptHighlighter(document());
   highlighter = NULL;
}

int CodeEditor::lineNumberAreaWidth()
{
   int digits = 1;
   int max = qMax(1, blockCount());
   while (max >= 10)
   {
      max /= 10;
      ++digits;
   }

   int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

   return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
   setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
   if (dy)
   {
      m_lineNumberArea->scroll(0, dy);
   }
   else if (m_countCache.first != blockCount() || m_countCache.second != textCursor().block().lineCount())
   {
      m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
      m_countCache.first = blockCount();
      m_countCache.second = textCursor().block().lineCount();
   }

   if (rect.contains(viewport()->rect()))
   {
      updateLineNumberAreaWidth(0);
   }
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
   QPlainTextEdit::resizeEvent(e);

   QRect cr = contentsRect();
   m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
   switch(e->key())
   {
   case Qt::Key_Tab:
      IndentLines(false);
      break;
   case Qt::Key_Backtab:
      IndentLines(true);
      break;
   default:
      QPlainTextEdit::keyPressEvent(e);
   }
}

void CodeEditor::IndentLines(bool backwards)
{
   if (textCursor().hasSelection())
   {
      //move selected blocks of text forward/backward ....
      int start = textCursor().selectionStart();
      int end = textCursor().selectionEnd() - 1;

      QTextCursor cursor(document());
      cursor.setPosition(start);
      int firstBlock = cursor.blockNumber();
      cursor.setPosition(end);
      int lastBlock = cursor.blockNumber();

      if (firstBlock == lastBlock)
      {
         //when selected on same line, delete it
         textCursor().removeSelectedText();
      }
      else
      {
         if (backwards)
         {
            cursor.beginEditBlock();
            for(int blockNumber = firstBlock; blockNumber <= lastBlock; ++ blockNumber)
            {
               auto block = document()->findBlockByNumber(blockNumber);
               cursor.setPosition(block.position());
               for(int spaces=0; spaces< m_tabSpaces && block.text().at(0) == ' '; ++spaces)
               {
                  cursor.deleteChar();
               }
            }
            cursor.endEditBlock();
         }
         else
         {
            QString text;
            for (int spaces = 0; spaces < m_tabSpaces; ++spaces)
            {
               text.append(' ');
            }
            cursor.beginEditBlock();
            for(int blockNumber = firstBlock; blockNumber <= lastBlock; ++ blockNumber)
            {
               auto block = document()->findBlockByNumber(blockNumber);
               cursor.setPosition(block.position());
               cursor.insertText(text);
            }
            cursor.endEditBlock();
         }
      }
   }
   else
   {
      //no selection . . . based on active cursor location
      //shift block text forward/backwards to next/prev tab space location
      if (backwards)
      {
         int spaces = textCursor().positionInBlock() % m_tabSpaces;
         if (spaces == 0)
         {
            spaces = m_tabSpaces;
         }
         textCursor().beginEditBlock();
         for (int pos = textCursor().positionInBlock() - 1; spaces > 0 && !textCursor().atBlockStart() && textCursor().block().text().at(pos) == ' '; --spaces, --pos)
         {
            textCursor().deletePreviousChar();
         }
         textCursor().endEditBlock();
      }
      else
      {
         QString text;
         for (int spaces = m_tabSpaces - (textCursor().positionInBlock() % m_tabSpaces); spaces > 0; --spaces)
         {
            text.append(' ');
         }
         textCursor().insertText(text);
      }
   }
}

void CodeEditor::highlightCurrentLine()
{
   /*
   This code highlights the line . . . we don't want that.
   QList<QTextEdit::ExtraSelection> extraSelections;

   if (!isReadOnly())
   {
      QTextEdit::ExtraSelection selection;

      QColor lineColor = QColor(Qt::yellow).lighter(160);

      selection.format.setBackground(lineColor);
      selection.format.setProperty(QTextFormat::FullWidthSelection, true);
      selection.cursor = textCursor();
      selection.cursor.clearSelection();
      extraSelections.append(selection);
   }

   setExtraSelections(extraSelections);
   */

   //highlight line number
   //m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
   m_lineNumberArea->update();
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
   QPainter painter(m_lineNumberArea);
   painter.fillRect(event->rect(), palette().color(QPalette::Window));


   QTextBlock block = firstVisibleBlock();
   int blockNumber = block.blockNumber();
   int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
   int bottom = top + (int) blockBoundingRect(block).height();

   int currentLine = textCursor().blockNumber();
   while (block.isValid() && top <= event->rect().bottom())
   {
      if (block.isVisible() && bottom >= event->rect().top())
      {
         QString number = QString::number(blockNumber + 1);
         if (blockNumber == currentLine)
         {
            painter.setPen(palette().color(QPalette::BrightText));
         }
         else
         {
            painter.setPen(palette().color(QPalette::Text));
         }
         painter.drawText(0, top, m_lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
      }

      block = block.next();
      top = bottom;
      bottom = top + (int) blockBoundingRect(block).height();
      ++blockNumber;
   }
}

JavaScriptHighlighter::JavaScriptHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::magenta);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bvar\\b" << "\\bfunction\\b"
    << "\\breturn\\b" << "\\bif\\b" << "\\bundefined\\b"
    << "\\belse\\b" << "\\bfor\\b" << "\\bswitch\\b"
    << "\\bcase\\b" << "\\bbreak\\b" << "\\bwhile\\b"
    << "\\bclass\\b" << "\\bcatch\\b" << "\\bconst\\b"
    << "\\bcontinue\\b" << "\\bdebugger\\b" << "\\bdefault\\b"
    << "\\bdelete\\b" << "\\bdo\\b" << "\\bexport\\b" << "\\bextends\\b"
    << "\\bfinally\\b" << "\\bimport\\b" << "\\binstanceof\\b"
    << "\\blet\\b" << "\\bnew\\b" << "\\bsuper\\b" << "\\bthis\\b"
    << "\\bthrow\\b" << "\\btry\\b" << "\\btypeof\\b"
    << "\\bvoid\\b" << "\\bwith\\b" << "\\byield\\b"
    << "\\bnull\\b" << "\\btrue\\b" << "\\bfalse\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    functionFormat.setForeground(QColor(148,0,211)); //purple
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\"[^\"]*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::darkGreen);
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");

}

void JavaScriptHighlighter::highlightBlock(const QString &text)
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

}
