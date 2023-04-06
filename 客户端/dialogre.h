#ifndef DIALOGRE_H
#define DIALOGRE_H

#include <QDialog>
#include <QWidget>
#include <QAbstractSocket>
namespace Ui {
class DialogRe;
}

class DialogRe : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRe(QWidget *parent = nullptr);
    ~DialogRe();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
private slots:
    void reciveRe();

signals:
    void showLogin();


private:
    Ui::DialogRe *ui;
    const QString REGIREQ = "2";         //注册标记
};

#endif // DIALOGRE_H
