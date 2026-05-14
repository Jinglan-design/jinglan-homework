#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include<QComboBox>
#include <QCheckBox>
#include<QDialog>
#include "minefield.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewGame();
    void onGameOver(bool win);
    void onMineCountChanged(int count);
    void onDifficultyChanged(int index);
    void onCustomGame();
    void onSoundToggled(bool checked);

private:
    void setupUI();
    void startGame(int rows,int cols,int mineCount);

    MineField *m_mineField;
    QLabel *m_mineCountLabel;
    QPushButton *m_newGameBtn;
    QComboBox *m_difficultyCombo;
    QPushButton *m_customBtn;
    QCheckBox *m_soundCheckBox;
};

#endif
