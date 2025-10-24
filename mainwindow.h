#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QKeyEvent>
#include <QtGlobal>

#include <cmath>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    double storedValue = 0.0; // 存储第一个操作数
    QString pendingOp;       // 待执行的二元运算符（+ - * /）
    bool waitingForOperand = false; // 标记是否等待下一个操作数

    // 辅助方法
    void abortOperation();
    void applyBinaryOperation(const QString &op, double rightOperand);
    void handleBinaryOperator(const QString &op);

private slots:
    void digitClicked();
    void pointClicked();
    void changeSignClicked();
    void backspaceClicked();
    void clearClicked();     // C 清除所有
    void clearEntryClicked(); // CE 清除当前输入
    void additiveOperatorClicked(); // 加减
    void multiplicativeOperatorClicked(); // 乘除
    void equalClicked();
    void unaryOperatorClicked(); // 1/x, x^2, √
    void percentClicked();
    
protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    QLabel *exprLabel = nullptr; // 显示运算过程的标签，位于输入框上方

    void updateExpressionDisplay(const QString &left, const QString &op = QString(), const QString &right = QString());
};
#endif // MAINWINDOW_H
