#ifndef MINEFIELD_H
#define MINEFIELD_H

#include <QWidget>
#include <QGridLayout>
#include <vector>
#include <ctime>
#include <cstdlib>
#include "cellbutton.h"

class MineField : public QWidget
{
    Q_OBJECT

public:
    explicit MineField(QWidget *parent = nullptr);
    void newGame(int rows = 9, int cols = 9, int mineCount = 10);
    static void setSoundEnabled(bool enabled);
    static void initSoundEffects();

signals:
    void gameOver(bool win);
    void mineCountChanged(int count);

private slots:
    void onCellLeftClick(int x, int y);
    void onCellRightClick(int x, int y);

private:
    void initGrid();
    void placeMines(int firstX = -1, int firstY = -1);
    void calculateNumbers();
    void revealCell(int x, int y);
    void revealAllMines();
    void checkWin();
    void resetAllButtons();
    void playLoseSound();   // 失败音效
    void playWinSound();

    QGridLayout *m_layout;
    std::vector<std::vector<CellButton*>> m_grid;
    std::vector<std::vector<bool>> m_isMine;
    std::vector<std::vector<int>> m_adjacentMines;
    std::vector<std::vector<bool>> m_revealed;
    std::vector<std::vector<bool>> m_flagged;

    int m_rows;
    int m_cols;
    int m_mineCount;
    int m_revealedCount;
    int m_flagCount;
    bool m_gameOver;
    bool m_win;
    bool m_firstMove;
    static class QSoundEffect* s_winSound;
    static class QSoundEffect* s_loseSound;
    static bool s_soundEnabled;
};

#endif