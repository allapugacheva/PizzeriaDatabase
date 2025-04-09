#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QStringList>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QTableView>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QStringList queryList;
    QStringList queryNameList;

    void executeQuery(QString query);
    void saveToFile(QAbstractItemModel *model, const QString& filePath);

private slots:
    void onExecuteButtonCliked();
    void onDeleteButtonClicked();
    void onExecuteNewButtonClicked();
    void onAddButtonClicked();
    void onSaveButtonClicked();
};

#endif // MAINWINDOW_H
