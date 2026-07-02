#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnRegistrar_clicked();

    void on_btnSalida_clicked();

    void on_btnBorrar_clicked();

    void on_btnBuscar_clicked();

    void on_btnGuardarNovedad_clicked();

    void on_btnVerNovedades_clicked();

    void on_btnBorrarNovedad_clicked();

private:
    Ui::MainWindow *ui;
       void cargarUsuarios();
};
#endif // MAINWINDOW_H
