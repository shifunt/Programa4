#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QDebug>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCompleter>
#include <QStringListModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // --- LOGIN ---
    bool accesoConcedido = false;
    int intentos = 0;
    QString claveCorrecta = "1234";

    while (!accesoConcedido && intentos < 3) {
        QString password = QInputDialog::getText(this, "Acceso Restringido",
                                                 "Ingresá la contraseña de Guardia:",
                                                 QLineEdit::Password);
        if (password == claveCorrecta) {
            accesoConcedido = true;
        } else {
            intentos++;
            if (intentos < 3) {
                QMessageBox::critical(this, "Error", "Contraseña incorrecta. Intentos restantes: " + QString::number(3 - intentos));
            } else {
                QMessageBox::critical(this, "Bloqueado", "Demasiados intentos fallidos. La app se cerrará.");
                QTimer::singleShot(0, this, &QWidget::close);
                return;
            }
        }
    }

    ui->setupUi(this);

    // --- CONEXIÓN A LA BASE DE DATOS (RUTA PORTABLE) ---
    // Antes: ruta fija de una sola PC (ej: "/home/shifu/Escritorio/SeguridadGolden/seguridadGael.db")
    // Ahora: se arma dinámicamente en la carpeta "Documentos" del usuario actual,
    // funcionando igual en Windows, Linux o Mac sin importar el nombre de usuario.
    QString rutaCarpeta = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/SeguridadGolden";
    QDir().mkpath(rutaCarpeta); // crea la carpeta si no existe

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(rutaCarpeta + "/seguridadGael.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Error de DB", "No se pudo conectar: " + db.lastError().text());
        qDebug() << "❌ Error de conexión:" << db.lastError().text();
        return;
    } else {
        qDebug() << "✅ Conexión exitosa a la base de datos.";
        qDebug() << "📂 Ruta de la base de datos:" << (rutaCarpeta + "/seguridadGael.db");
    }

    // --- CREAR TABLA CON TODAS LAS COLUMNAS (si no existe) ---
    QSqlQuery queryCrear;
    queryCrear.exec("CREATE TABLE IF NOT EXISTS usuarios ("
                    "patente TEXT PRIMARY KEY, "
                    "nombre TEXT, "
                    "apellido TEXT, "
                    "visita TEXT, "
                    "tipo TEXT, "
                    "hora TEXT, "
                    "fecha TEXT, "
                    "salida TEXT)");

    // --- AGREGAR COLUMNAS FALTANTES SI LA TABLA YA EXISTÍA SIN ELLAS ---
    // Esto permite migrar bases de datos viejas sin borrarlas
    QSqlQuery queryAlter;
    queryAlter.exec("ALTER TABLE usuarios ADD COLUMN hora TEXT");
    queryAlter.exec("ALTER TABLE usuarios ADD COLUMN fecha TEXT");
    queryAlter.exec("ALTER TABLE usuarios ADD COLUMN salida TEXT");
    // (SQLite ignora el error si la columna ya existe, no es necesario chequear)

    // --- CARGAR REGISTROS EXISTENTES EN LA TABLA VISUAL ---
    cargarUsuarios();

    // --- AUTOCOMPLETADO POR PATENTE ---
    QStringList listaPatentes;
    QSqlQuery queryPatentes("SELECT patente FROM usuarios");
    while (queryPatentes.next()) {
        listaPatentes << queryPatentes.value(0).toString();
    }

    QStringListModel *modeloCompleter = new QStringListModel(listaPatentes, this);
    QCompleter *completer = new QCompleter(modeloCompleter, this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->txtPatente->setCompleter(completer);

    connect(completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, [this](const QString &patenteSeleccionada) {
                QSqlQuery query;
                query.prepare("SELECT nombre, apellido, visita, tipo FROM usuarios WHERE patente = :patente");
                query.bindValue(":patente", patenteSeleccionada);
                if (query.exec() && query.next()) {
                    ui->txtNombre->setText(query.value(0).toString());
                    ui->txtApellido->setText(query.value(1).toString());
                    ui->txtVisita->setText(query.value(2).toString());
                    ui->chkPropietario->setChecked(query.value(3).toString() == "PROPIETARIO");
                }
            });

    // --- ESTILOS ---
    this->setWindowTitle("SEGURIDAD GOLDEN - Sistema de Control");
    this->setStyleSheet("background-color: #121212; color: #e0e0e0; font-family: 'Segoe UI', Arial;");

    QString estiloBotones = "QPushButton { background-color: #1b5e20; color: white; border: 1px solid #2e7d32; "
                            "border-radius: 4px; font-weight: bold; min-height: 35px; } "
                            "QPushButton:hover { background-color: #2e7d32; }";
    ui->btnRegistrar->setStyleSheet(estiloBotones);
    ui->btnBuscar->setStyleSheet("background-color: #0d47a1; color: white; border-radius: 4px; font-weight: bold;");
    ui->btnBorrar->setStyleSheet("background-color: #b71c1c; color: white; border-radius: 4px; font-weight: bold;");

    QString estiloInputs = "QLineEdit { background-color: #1e1e1e; border: 1px solid #333333; "
                           "border-radius: 4px; padding: 5px; color: #00ff00; }";
    ui->txtNombre->setStyleSheet(estiloInputs);
    ui->txtApellido->setStyleSheet(estiloInputs);
    ui->txtPatente->setStyleSheet(estiloInputs);
    ui->txtVisita->setStyleSheet(estiloInputs);

    ui->tablaDatos->setStyleSheet(
        "QTableWidget { background-color: #121212; alternate-background-color: #1a1a1a; "
        "gridline-color: #333333; color: #ffffff; selection-background-color: #444444; } "
        "QHeaderView::section { background-color: #000000; color: #00ff00; padding: 5px; border: 1px solid #333333; }");
    ui->tablaDatos->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// -------------------------------------------------------------------
// REGISTRAR ENTRADA
// -------------------------------------------------------------------
void MainWindow::on_btnRegistrar_clicked()
{
    QString hora    = QTime::currentTime().toString("HH:mm");
    QString fecha   = QDate::currentDate().toString("yyyy-MM-dd");
    QString nombre   = ui->txtNombre->text().toUpper().trimmed();
    QString apellido = ui->txtApellido->text().toUpper().trimmed();
    QString patente  = ui->txtPatente->text().toUpper().trimmed();
    QString visita   = ui->txtVisita->text().toUpper().trimmed();
    QString tipo     = ui->chkPropietario->isChecked() ? "PROPIETARIO" : "VISITA";

    if (patente.isEmpty()) {
        QMessageBox::warning(this, "Atención", "La patente no puede estar vacía.");
        return;
    }

    // INSERT si no existe, UPDATE si ya existe
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO usuarios (patente, nombre, apellido, visita, tipo, hora, fecha, salida) "
                  "VALUES (:patente, :nombre, :apellido, :visita, :tipo, :hora, :fecha, '')");
    query.bindValue(":patente", patente);
    query.bindValue(":nombre",  nombre);
    query.bindValue(":apellido",apellido);
    query.bindValue(":visita",  visita);
    query.bindValue(":tipo",    tipo);
    query.bindValue(":hora",    hora);
    query.bindValue(":fecha",   fecha);

    if (!query.exec()) {
        qDebug() << "❌ Error al guardar:" << query.lastError().text();
        QMessageBox::critical(this, "Error", "No se pudo guardar el registro.");
        return;
    } else {
        qDebug() << "✅ Registro guardado correctamente.";
    }

    // Actualizar completer
    if (ui->txtPatente->completer()) {
        QStringListModel *mod = qobject_cast<QStringListModel*>(ui->txtPatente->completer()->model());
        if (mod) {
            QStringList lista = mod->stringList();
            if (!lista.contains(patente)) {
                lista << patente;
                mod->setStringList(lista);
            }
        }
    }

    // Mostrar en tabla visual
    int fila = ui->tablaDatos->rowCount();
    ui->tablaDatos->insertRow(fila);
    ui->tablaDatos->setItem(fila, 0, new QTableWidgetItem(hora));
    ui->tablaDatos->setItem(fila, 1, new QTableWidgetItem(nombre));
    ui->tablaDatos->setItem(fila, 2, new QTableWidgetItem(apellido));
    ui->tablaDatos->setItem(fila, 3, new QTableWidgetItem(patente));
    ui->tablaDatos->setItem(fila, 4, new QTableWidgetItem(visita));
    ui->tablaDatos->setItem(fila, 5, new QTableWidgetItem(tipo));
    ui->tablaDatos->setItem(fila, 6, new QTableWidgetItem(""));  // salida vacía

    // Guardar en CSV de respaldo
    QString ruta = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/SeguridadGolden";
    QDir().mkpath(ruta);
    QFile archivo(ruta + "/registros.csv");
    if (archivo.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream salida(&archivo);
        salida << fecha << "," << hora << "," << nombre << "," << apellido << ","
               << patente << "," << visita << "," << tipo << "\n";
        archivo.close();
    }

    // Limpiar campos
    ui->txtNombre->clear();
    ui->txtApellido->clear();
    ui->txtPatente->clear();
    ui->txtVisita->clear();
    ui->chkPropietario->setChecked(false);
}

// -------------------------------------------------------------------
// REGISTRAR SALIDA
// -------------------------------------------------------------------
void MainWindow::on_btnSalida_clicked()
{
    int filaSeleccionada = ui->tablaDatos->currentRow();

    if (filaSeleccionada == -1) {
        QMessageBox::warning(this, "Atención", "Seleccioná una fila de la tabla para registrar la salida.");
        return;
    }

    QTableWidgetItem *patenteItem = ui->tablaDatos->item(filaSeleccionada, 3);
    if (!patenteItem || patenteItem->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Atención", "No se encontró la patente en la fila seleccionada.");
        return;
    }

    QString patente    = patenteItem->text().trimmed().toUpper();
    QString horaSalida = QTime::currentTime().toString("HH:mm");
    QString fechaSalida = QDate::currentDate().toString("yyyy-MM-dd");

    // Actualizar en la base de datos
    QSqlQuery query;
    query.prepare("UPDATE usuarios SET salida = :salida, fecha = :fecha WHERE patente = :patente");
    query.bindValue(":salida",  horaSalida);
    query.bindValue(":fecha",   fechaSalida);
    query.bindValue(":patente", patente);

    if (!query.exec()) {
        qDebug() << "❌ Error al actualizar salida:" << query.lastError().text();
        QMessageBox::critical(this, "Error", "No se pudo registrar la salida en la base de datos.");
        return;
    } else {
        qDebug() << "✅ Salida registrada en la base.";
    }

    // Actualizar la celda de salida en la tabla visual (columna 6)
    ui->tablaDatos->setItem(filaSeleccionada, 6, new QTableWidgetItem(horaSalida));

    // Resaltar fila en azul para indicar que ya salió
    for (int i = 0; i < ui->tablaDatos->columnCount(); i++) {
        QTableWidgetItem *item = ui->tablaDatos->item(filaSeleccionada, i);
        if (item) {
            item->setBackground(QColor("#0d47a1"));
            item->setForeground(Qt::white);
            QFont font = item->font();
            font.setItalic(true);
            item->setFont(font);
        }
    }

    ui->tablaDatos->clearSelection();
    // ✅ NO llamamos cargarUsuarios() acá — eso pisaba la tabla con datos viejos
}

// -------------------------------------------------------------------
// BORRAR REGISTRO
// -------------------------------------------------------------------
void MainWindow::on_btnBorrar_clicked()
{
    int fila = ui->tablaDatos->currentRow();

    if (fila == -1) {
        QMessageBox::warning(this, "Atención", "Por favor, seleccioná la fila que querés borrar.");
        return;
    }

    auto respuesta = QMessageBox::question(this, "Confirmar",
                                           "¿Estás seguro de que querés eliminar este registro?",
                                           QMessageBox::Yes | QMessageBox::No);

    if (respuesta == QMessageBox::Yes) {
        // Borrar de la base de datos
        QTableWidgetItem *patenteItem = ui->tablaDatos->item(fila, 3);
        if (patenteItem && !patenteItem->text().trimmed().isEmpty()) {
            QString patente = patenteItem->text().trimmed().toUpper();
            QSqlQuery query;
            query.prepare("DELETE FROM usuarios WHERE patente = :patente");
            query.bindValue(":patente", patente);
            if (!query.exec()) {
                qDebug() << "❌ Error al borrar de DB:" << query.lastError().text();
            } else {
                qDebug() << "✅ Registro borrado de la base.";
            }
        }

        // Borrar de la tabla visual
        ui->tablaDatos->removeRow(fila);

        // Reescribir CSV desde lo que queda en la tabla
        QString ruta = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                       + "/SeguridadGolden/registros.csv";
        QFile archivo(ruta);
        if (archivo.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream salida(&archivo);
            for (int i = 0; i < ui->tablaDatos->rowCount(); ++i) {
                auto cell = [&](int col) -> QString {
                    QTableWidgetItem *it = ui->tablaDatos->item(i, col);
                    return it ? it->text() : "";
                };
                salida << QDate::currentDate().toString("yyyy-MM-dd") << ","
                       << cell(0) << "," << cell(1) << "," << cell(2) << ","
                       << cell(3) << "," << cell(4) << "," << cell(5) << "\n";
            }
            archivo.close();
        }

        QMessageBox::information(this, "Éxito", "Registro eliminado correctamente.");
    }
}

// -------------------------------------------------------------------
// BUSCAR POR PATENTE
// -------------------------------------------------------------------
void MainWindow::on_btnBuscar_clicked()
{
    QString patenteBusqueda = ui->txtPatente->text().toUpper().trimmed();

    if (patenteBusqueda.isEmpty()) {
        QMessageBox::information(this, "Buscador", "Ingresá una patente para buscar.");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT nombre, apellido, visita, tipo FROM usuarios WHERE patente = :patente");
    query.bindValue(":patente", patenteBusqueda);

    if (query.exec() && query.next()) {
        ui->txtNombre->setText(query.value(0).toString());
        ui->txtApellido->setText(query.value(1).toString());
        ui->txtVisita->setText(query.value(2).toString());
        ui->chkPropietario->setChecked(query.value(3).toString() == "PROPIETARIO");
        QMessageBox::information(this, "Buscador", "¡Datos recuperados de la Base de Datos!");
    } else {
        QMessageBox::information(this, "Buscador", "Esta patente no tiene registros previos.");
    }
}

// -------------------------------------------------------------------
// GUARDAR NOVEDAD
// -------------------------------------------------------------------
void MainWindow::on_btnGuardarNovedad_clicked()
{
    QString novedad = ui->txtNovedad->toPlainText().trimmed();

    if (novedad.isEmpty()) {
        QMessageBox::warning(this, "Atención", "Escribí algo en la novedad antes de guardar.");
        return;
    }

    QString ruta = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/SeguridadGolden";
    QDir().mkpath(ruta);
    QFile archivo(ruta + "/novedades.txt");

    if (archivo.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream salida(&archivo);
        QString fechaHora = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss");
        salida << "[" << fechaHora << "] " << novedad << "\n";
        salida << "------------------------------------------\n";
        archivo.close();
        ui->txtNovedad->clear();
        QMessageBox::information(this, "Éxito", "Novedad registrada en el libro de guardia.");
    } else {
        QMessageBox::critical(this, "Error", "No se pudo abrir el libro de novedades.");
    }
}

// -------------------------------------------------------------------
// VER NOVEDADES
// -------------------------------------------------------------------
void MainWindow::on_btnVerNovedades_clicked()
{
    QString ruta = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                   + "/SeguridadGolden/novedades.txt";
    QFile archivo(ruta);

    if (!archivo.exists()) {
        QMessageBox::information(this, "Libro de Guardia", "Aún no hay novedades registradas.");
        return;
    }

    if (archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString todoElContenido = archivo.readAll();
        archivo.close();

        QDialog *ventanaNovedades = new QDialog(this);
        ventanaNovedades->setMinimumSize(400, 500);
        ventanaNovedades->setWindowTitle("Historial de Novedades");
        ventanaNovedades->setStyleSheet("background-color: #121212; color: #00ff00;");

        QTextEdit *visorTexto = new QTextEdit(ventanaNovedades);
        visorTexto->setReadOnly(true);
        visorTexto->setPlainText(todoElContenido);
        visorTexto->setStyleSheet("background-color: #1e1e1e; border: 1px solid #333333; font-family: Consolas;");

        QVBoxLayout *layout = new QVBoxLayout(ventanaNovedades);
        layout->addWidget(visorTexto);
        ventanaNovedades->setLayout(layout);
        ventanaNovedades->show();
    } else {
        QMessageBox::critical(this, "Error", "No se pudo abrir el archivo de novedades.");
    }
}

// -------------------------------------------------------------------
// BORRAR NOVEDADES
// -------------------------------------------------------------------
void MainWindow::on_btnBorrarNovedad_clicked()
{
    auto respuesta = QMessageBox::question(this, "Confirmar Limpieza",
                                           "¿Estás seguro de que querés vaciar por completo el libro de novedades?",
                                           QMessageBox::Yes | QMessageBox::No);

    if (respuesta == QMessageBox::Yes) {
        QString ruta = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                       + "/SeguridadGolden/novedades.txt";
        QFile archivo(ruta);
        if (archivo.open(QIODevice::WriteOnly | QIODevice::Text)) {
            archivo.close();
            QMessageBox::information(this, "Éxito", "El libro de novedades ha sido vaciado correctamente.");
        } else {
            QMessageBox::critical(this, "Error", "No se pudo acceder al archivo para borrar las novedades.");
        }
    }
}

// -------------------------------------------------------------------
// CARGAR USUARIOS DESDE LA BASE DE DATOS
// -------------------------------------------------------------------
void MainWindow::cargarUsuarios()
{
    ui->tablaDatos->setRowCount(0);

    QSqlQuery query("SELECT hora, nombre, apellido, patente, visita, tipo, salida FROM usuarios ORDER BY fecha DESC, hora DESC");
    while (query.next()) {
        int row = ui->tablaDatos->rowCount();
        ui->tablaDatos->insertRow(row);

        ui->tablaDatos->setItem(row, 0, new QTableWidgetItem(query.value("hora").toString()));
        ui->tablaDatos->setItem(row, 1, new QTableWidgetItem(query.value("nombre").toString()));
        ui->tablaDatos->setItem(row, 2, new QTableWidgetItem(query.value("apellido").toString()));
        ui->tablaDatos->setItem(row, 3, new QTableWidgetItem(query.value("patente").toString()));
        ui->tablaDatos->setItem(row, 4, new QTableWidgetItem(query.value("visita").toString()));
        ui->tablaDatos->setItem(row, 5, new QTableWidgetItem(query.value("tipo").toString()));
        ui->tablaDatos->setItem(row, 6, new QTableWidgetItem(query.value("salida").toString()));

        // Si tiene salida registrada, pintar la fila en azul
        if (!query.value("salida").toString().isEmpty()) {
            for (int col = 0; col < ui->tablaDatos->columnCount(); col++) {
                QTableWidgetItem *item = ui->tablaDatos->item(row, col);
                if (item) {
                    item->setBackground(QColor("#0d47a1"));
                    item->setForeground(Qt::white);
                    QFont font = item->font();
                    font.setItalic(true);
                    item->setFont(font);
                }
            }
        }
    }
}
