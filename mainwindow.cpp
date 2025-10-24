#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QDebug>
#include <QKeyEvent>
#include <QEvent>
#include <QLabel>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 连接数字按钮
    connect(ui->btmNum0, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btmNum1, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btmNum2, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btmNum3, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btmNum4, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btmNum5, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btmNum6, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btmNum7, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btmNum8, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btmNum9, &QPushButton::clicked, this, &MainWindow::digitClicked);

    // 小数点
    connect(ui->pushButton_22, &QPushButton::clicked, this, &MainWindow::pointClicked);

    // 符号切换 ±
    connect(ui->pushButton_24, &QPushButton::clicked, this, &MainWindow::changeSignClicked);

    // 删除、清除、全部清除
    connect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::backspaceClicked); // DEL
    connect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::clearClicked); // C
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::clearEntryClicked); // CE

    // 运算符 + - * /
    connect(ui->pushButton_19, &QPushButton::clicked, this, &MainWindow::additiveOperatorClicked); // +
    connect(ui->pushButton_15, &QPushButton::clicked, this, &MainWindow::additiveOperatorClicked); // -
    connect(ui->pushButton_11, &QPushButton::clicked, this, &MainWindow::multiplicativeOperatorClicked); // ✖
    connect(ui->pushButton_7, &QPushButton::clicked, this, &MainWindow::multiplicativeOperatorClicked); // ➗

    // 等号
    connect(ui->pushButton_23, &QPushButton::clicked, this, &MainWindow::equalClicked);

    // 一元运算: 1/x, x^2, √
    connect(ui->pushButton_8, &QPushButton::clicked, this, &MainWindow::unaryOperatorClicked); // 1/x
    connect(ui->pushButton_5, &QPushButton::clicked, this, &MainWindow::unaryOperatorClicked); // x^2
    connect(ui->pushButton_6, &QPushButton::clicked, this, &MainWindow::unaryOperatorClicked); // √

    // 百分号
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::percentClicked); // %

    // 初始化显示
    ui->lineEdit->setText("0");

    // 拦截 lineEdit 的按键事件，防止默认的文本插入（我们用自定义逻辑处理）
    ui->lineEdit->installEventFilter(this);

    // 在 lineEdit 上方插入用于显示运算过程的标签
    exprLabel = new QLabel(this);
    exprLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QFont f = exprLabel->font();
    f.setPointSize(10);
    exprLabel->setFont(f);
    exprLabel->setText("");
    QVBoxLayout *vlay = qobject_cast<QVBoxLayout *>(ui->centralwidget->layout());
    if (vlay) vlay->insertWidget(0, exprLabel);

    // 简单样式表，改善界面视觉（可按需调整）
    QString style = R"(
        QLineEdit { font-size: 24px; padding: 6px; }
        QLabel { color: #666; font-size: 12px; }
        QPushButton { font-size: 18px; min-width: 48px; min-height: 36px; background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #ffffff, stop:1 #d0d0d0); border: 1px solid #888; border-radius: 6px; }
        QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #f5f5f5, stop:1 #cfcfcf); }
        QPushButton:pressed { background: #bfbfbf; }
    )";
    this->setStyleSheet(style);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 辅助：中止并显示错误
void MainWindow::abortOperation()
{
    ui->lineEdit->setText("Error");
    storedValue = 0.0;
    pendingOp.clear();
    waitingForOperand = false;
}

void MainWindow::applyBinaryOperation(const QString &op, double rightOperand)
{
    double result = 0.0;
    if (op == "+") result = storedValue + rightOperand;
    else if (op == "-") result = storedValue - rightOperand;
    else if (op == "*" || op == "✖") result = storedValue * rightOperand;
    else if (op == "/" || op == "➗") {
        if (std::abs(rightOperand) < 1e-12) { // rightOperand == 0
            abortOperation();
            return;
        }
        result = storedValue / rightOperand;
    } else {
        return; // 不支持的运算
    }

    // 将结果显示并更新状态
    ui->lineEdit->setText(QString::number(result));
    storedValue = result;
    pendingOp.clear();
    waitingForOperand = true;
}

void MainWindow::handleBinaryOperator(const QString &op)
{
    // 将可能的符号统一为 + - * /
    QString normalized = op;
    if (normalized == "✖") normalized = "*";
    if (normalized == "➗") normalized = "/";

    double operand = ui->lineEdit->text().toDouble();
    if (!pendingOp.isEmpty()) {
        applyBinaryOperation(pendingOp, operand);
    } else {
        storedValue = operand;
    }

    pendingOp = normalized;
    waitingForOperand = true;
    if (exprLabel) {
        QString dispOp = pendingOp;
        if (dispOp == "*") dispOp = "✖";
        if (dispOp == "/") dispOp = "➗";
        exprLabel->setText(QString::number(storedValue) + " " + dispOp);
    }
}

// 数字按钮处理
void MainWindow::digitClicked()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    if (!clickedButton) return;

    QString digit = clickedButton->text();
    QString displayText = ui->lineEdit->text();

    if (displayText == "0" || waitingForOperand || displayText == "Error") {
        ui->lineEdit->setText(digit);
        waitingForOperand = false;
    } else {
        ui->lineEdit->setText(displayText + digit);
    }
}

void MainWindow::pointClicked()
{
    QString displayText = ui->lineEdit->text();
    if (waitingForOperand) {
        ui->lineEdit->setText("0.");
        waitingForOperand = false;
        return;
    }
    if (!displayText.contains('.')) {
        ui->lineEdit->setText(displayText + '.');
    }
}

void MainWindow::changeSignClicked()
{
    QString text = ui->lineEdit->text();
    if (text == "0" || text == "Error") return;
    if (text.startsWith('-')) text.remove(0,1);
    else text.prepend('-');
    ui->lineEdit->setText(text);
}

void MainWindow::backspaceClicked()
{
    if (waitingForOperand) return;
    QString text = ui->lineEdit->text();
    if (text == "Error") { ui->lineEdit->setText("0"); return; }
    text.chop(1);
    if (text.isEmpty() || text == "-") ui->lineEdit->setText("0");
    else ui->lineEdit->setText(text);
}

void MainWindow::clearClicked()
{
    // C: 清除所有
    ui->lineEdit->setText("0");
    storedValue = 0.0;
    pendingOp.clear();
    waitingForOperand = false;
    if (exprLabel) exprLabel->setText("");
}

void MainWindow::clearEntryClicked()
{
    // CE: 清除当前输入
    ui->lineEdit->setText("0");
    waitingForOperand = true;
}

void MainWindow::additiveOperatorClicked()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    if (!clickedButton) return;
    QString clickedOp = clickedButton->text(); // + or -
    handleBinaryOperator(clickedOp);
}

void MainWindow::multiplicativeOperatorClicked()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    if (!clickedButton) return;
    QString clickedOp = clickedButton->text(); // ✖ or ➗
    handleBinaryOperator(clickedOp);
}

void MainWindow::equalClicked()
{
    double rhs = ui->lineEdit->text().toDouble();
    if (!pendingOp.isEmpty()) {
        // 显示完整运算过程，例如 "12 + 3 ="
        QString dispOp = pendingOp;
        if (dispOp == "*") dispOp = "✖";
        if (dispOp == "/") dispOp = "➗";
        if (exprLabel) exprLabel->setText(QString::number(storedValue) + " " + dispOp + " " + QString::number(rhs) + " =");

        applyBinaryOperation(pendingOp, rhs);
    } else {
        // 没有运算符，直接显示当前值
        ui->lineEdit->setText(QString::number(rhs));
    }
}

void MainWindow::unaryOperatorClicked()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    if (!clickedButton) return;
    QString op = clickedButton->text();
    double val = ui->lineEdit->text().toDouble();

    if (op == "1/x") {
        if (qFuzzyCompare(val + 1.0, 1.0)) { // val == 0
            abortOperation();
            return;
        }
        val = 1.0 / val;
    } else if (op == "x²") {
        val = val * val;
    } else if (op == "√") {
        if (val < 0) { abortOperation(); return; }
        val = std::sqrt(val);
    }

    ui->lineEdit->setText(QString::number(val));
    waitingForOperand = true;
}

void MainWindow::percentClicked()
{
    double val = ui->lineEdit->text().toDouble();
    val = val / 100.0;
    ui->lineEdit->setText(QString::number(val));
    waitingForOperand = true;
}

// 键盘支持：数字、小数点、运算符、回车、退格、C
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!event) return;

    int k = event->key();
    QString txt = event->text();

    // 如果有文本输入（兼容主键盘和小键盘），先处理可打印字符
    if (!txt.isEmpty()) {
        QChar ch = txt.at(0);

        // 数字
        if (ch.isDigit()) {
            QString digit = QString(ch);
            ui->lineEdit->setFocus();
            if (ui->lineEdit->text() == "0" || waitingForOperand || ui->lineEdit->text() == "Error") {
                ui->lineEdit->setText(digit);
                waitingForOperand = false;
            } else {
                ui->lineEdit->setText(ui->lineEdit->text() + digit);
            }
            return;
        }

        // 小数点/逗号
        if (ch == '.' || ch == ',') {
            pointClicked();
            return;
        }

        // 运算符字符 + - * /
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            handleBinaryOperator(QString(ch));
            return;
        }

        // 百分号
        if (ch == '%') {
            percentClicked();
            return;
        }

        // 字母 C 清除
        if (ch.toLower() == 'c') {
            clearClicked();
            return;
        }
    }

    // 处理非文本按键
    if (k == Qt::Key_Return || k == Qt::Key_Enter) {
        equalClicked();
        return;
    }

    if (k == Qt::Key_Backspace) {
        backspaceClicked();
        return;
    }

    if (k == Qt::Key_Escape) {
        clearClicked();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // 仅拦截我们关注的 lineEdit 的按键事件
    if (obj == ui->lineEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (!ke) return false;

        // 使用相同的按键处理逻辑，但拦截以阻止 QLineEdit 插入字符
        QString txt = ke->text();
        int k = ke->key();

        if (!txt.isEmpty()) {
            QChar ch = txt.at(0);
            if (ch.isDigit()) {
                QString digit = QString(ch);
                if (ui->lineEdit->text() == "0" || waitingForOperand || ui->lineEdit->text() == "Error") {
                    ui->lineEdit->setText(digit);
                    waitingForOperand = false;
                } else {
                    ui->lineEdit->setText(ui->lineEdit->text() + digit);
                }
                return true; // 已处理，阻止默认插入
            }

            if (ch == '.' || ch == ',') {
                pointClicked();
                return true;
            }

            if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
                handleBinaryOperator(QString(ch));
                return true;
            }

            if (ch == '%') {
                percentClicked();
                return true;
            }

            if (ch.toLower() == 'c') {
                clearClicked();
                return true;
            }
        }

        // 处理非文本键
        if (k == Qt::Key_Return || k == Qt::Key_Enter) {
            equalClicked();
            return true;
        }

        if (k == Qt::Key_Backspace) {
            backspaceClicked();
            return true;
        }

        if (k == Qt::Key_Escape) {
            clearClicked();
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

