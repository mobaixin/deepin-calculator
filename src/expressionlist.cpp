#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>
#include "dthememanager.h"
#include "expressionlist.h"
#include "utils.h"
#include "abacus/Expression.h"
#include <QDebug>

DWIDGET_USE_NAMESPACE

ExpressionList::ExpressionList(QWidget *parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    listView = new ListView;
    inputEdit = new InputEdit;

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(listView);
    layout->addWidget(inputEdit);

    inputEdit->setTextMargins(10, 0, 10, 8);
    inputEdit->setFixedHeight(55);
    inputEdit->setAlignment(Qt::AlignRight);

    defaultFontSize = 25;
    minFontSize = 10;
    fontSize = defaultFontSize;
    isLeftBracket = true;
    isContinue = true;
    isAllClear = false;

    setFixedHeight(160);
    initFontSize();

    connect(inputEdit, &QLineEdit::textChanged, this, &ExpressionList::inputEditChanged);
}

ExpressionList::~ExpressionList()
{
}

void ExpressionList::enterNumberEvent(const QString &num)
{
    if (!isContinue || inputEdit->text() == "0") {
        inputEdit->setText("");
        isContinue = true;
    }

    inputEdit->insert(num);

    emit clearStateChanged(false);
}

void ExpressionList::enterPointEvent()
{
    if (lastCharIsSymbol()) {
        inputEdit->insert("0");
    }

    inputEdit->insert(".");
}

void ExpressionList::enterSymbolEvent(const QString &str)
{
    if (lastCharIsSymbol()) {
        enterBackspaceEvent();
    } else if (lastCharIsPoint() || lastCharIsLeftBracket()) {
        inputEdit->insert("0");
    } else if (str == "－" && inputEdit->text() == "0") {
        inputEdit->clear();
    }

    inputEdit->insert(str);
    isContinue = true;
    isAllClear = false;

    emit clearStateChanged(false);
}

void ExpressionList::enterBracketsEvent()
{

}

void ExpressionList::enterBackspaceEvent()
{
    inputEdit->backspace();

    isContinue = true;
}

void ExpressionList::enterClearEvent()
{
    if (isAllClear) {
        listView->clearItems();
        isAllClear = false;

        emit clearStateChanged(false);
    } else {
        inputEdit->setText("");
        isAllClear = true;

        initFontSize();
        emit clearStateChanged(true);
    }

}

void ExpressionList::enterEqualEvent()
{
    if (inputEdit->text() == "0" || inputEdit->text().isEmpty() || !isContinue || lastCharIsLeftBracket() || lastCharIsPoint()) {
        return;
    }

    Expression e(formatExp(inputEdit->text()).toStdString(), 10);

    try {
        const QString result = QString::number(e.getResult());

        if (inputEdit->text() == result) {
            return;
        }

        listView->addItem(inputEdit->text() + " = " + result);
        inputEdit->setText(result);

        isContinue = false;
    } catch (runtime_error err) {
        qDebug() << err.what();
    }
}

void ExpressionList::copyResultToClipboard()
{
    Expression e(formatExp(inputEdit->text()).toStdString(), 10);

    try {
        QApplication::clipboard()->setText(QString::number(e.getResult()));
    } catch (runtime_error err) {

    }
}

int ExpressionList::getItemsCount()
{
    return listView->getItemsCount();
}

void ExpressionList::inputEditChanged(const QString &text)
{
    // using setText() will move the cursor pos to end.
    const int currentPos = inputEdit->cursorPosition();

    // replace string.
    inputEdit->setText(QString(text).replace("+", "＋").replace("-", "－")
                                    .replace(QRegExp("[x|X|*]"), "×").replace("/", "÷")
                                    .replace("（", "(").replace("）", ")")
                                    .replace("。", "."));
    inputEdit->setCursorPosition(currentPos);

    // make font size of inputEdit fit text content.
    QFontMetrics fm = inputEdit->fontMetrics();
    int w = fm.boundingRect(inputEdit->text()).width();

    if (w >= inputEdit->width() - 30) {
        fontSize -= 2;
        QFont font;
        font.setPointSize(qMax(fontSize, minFontSize));
        inputEdit->setFont(font);
    }

    // get current input char.
    QChar currentInputChar = formatExp(inputEdit->text()).at(currentPos - 1);

    if (currentInputChar == '+' || currentInputChar == '-' || currentInputChar == '*' || currentInputChar == '/') {
        isContinue = true;
    } else if (currentInputChar == '0' || currentInputChar == '1' || currentInputChar == '2' || currentInputChar == '3' || currentInputChar == '4' || currentInputChar == '5' || currentInputChar == '6' || currentInputChar == '7' || currentInputChar == '8' || currentInputChar == '9' || currentInputChar == '.') {
        if (!isContinue) {
            inputEdit->clear();
            isContinue = true;
        }
    }

    // when text is empty, the clear button status will changed.
    if (text.isEmpty()) {
        isAllClear = true;
        initFontSize();
        emit clearStateChanged(true);
    } else {
        isAllClear = false;
        emit clearStateChanged(false);
    }
}

void ExpressionList::initFontSize()
{
    fontSize = defaultFontSize;
    QFont font;
    font.setPointSize(fontSize);
    inputEdit->setFont(font);
}

QString ExpressionList::formatExp(const QString &exp)
{
    return QString(exp).replace("＋", "+").replace("－", "-").replace("×", "*").replace("÷", "/");
}

QChar ExpressionList::getLastChar()
{
    QString exp = formatExp(inputEdit->text());
    QString::const_iterator laster = exp.end();
    laster--;

    return *laster;
}

bool ExpressionList::lastCharIsNumber()
{
    const QChar lastChar = getLastChar();

    if (lastChar == '0' || lastChar == '1' || lastChar == '2' || lastChar == '3' ||
        lastChar == '4' || lastChar == '5' || lastChar == '6' || lastChar == '7' ||
        lastChar == '8' || lastChar == '9') {
        return true;
    } else {
        return false;
    }
}

bool ExpressionList::lastCharIsSymbol()
{
    const QChar lastChar = getLastChar();

    if (lastChar == '+' || lastChar == '-' || lastChar == '*' || lastChar == '/') {
        return true;
    } else {
        return false;
    }
}

bool ExpressionList::lastCharIsPoint()
{
    return getLastChar() == '.' ? true : false;
}

bool ExpressionList::lastCharIsLeftBracket()
{
    return getLastChar() == '(' ? true : false;
}

bool ExpressionList::lastCharIsRightBracket()
{
    return getLastChar() == ')' ? true : false;
}
