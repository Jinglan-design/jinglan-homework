#include "minefield.h"
#include <QDebug>
#include <QRandomGenerator>
#include<queue>
#include <QSoundEffect>
#include <QUrl>
QSoundEffect* MineField::s_winSound = nullptr;
QSoundEffect* MineField::s_loseSound = nullptr;
bool MineField::s_soundEnabled = true;

MineField::MineField(QWidget *parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_rows(0)
    , m_cols(0)
    , m_mineCount(0)
    , m_revealedCount(0)
    , m_flagCount(0)
    , m_gameOver(false)
    , m_win(false)
    , m_firstMove(true)
{
    initSoundEffects();
    m_layout = new QGridLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);

    newGame();
}

void MineField::setSoundEnabled(bool enabled)
{
    s_soundEnabled = enabled;
}

void MineField::initSoundEffects()
{
    // 胜利音效
    if (s_winSound == nullptr) {
        s_winSound = new QSoundEffect();
        s_winSound->setSource(QUrl("qrc:/sounds/win.wav"));
        s_winSound->setVolume(0.6f);
        s_winSound->setLoopCount(1);
    }

    // 失败音效
    if (s_loseSound == nullptr) {
        s_loseSound = new QSoundEffect();
        s_loseSound->setSource(QUrl("qrc:/sounds/lose.wav"));
        s_loseSound->setVolume(0.6f);
        s_loseSound->setLoopCount(1);
    }
}

void MineField::playWinSound()
{
    if (s_soundEnabled && s_winSound && s_winSound->isLoaded()) {
        s_winSound->play();
    }
}

void MineField::playLoseSound()
{
    if (s_soundEnabled && s_loseSound && s_loseSound->isLoaded()) {
        s_loseSound->play();
    }
}
void MineField::newGame(int rows, int cols, int mineCount)
{
    qDebug() << "=== newGame START ===";

    // 参数限制
    rows = qBound(8, rows, 30);
    cols = qBound(8, cols, 40);
    mineCount = qBound(1, mineCount, rows * cols - 1);

    // 重要：先断开所有信号连接，避免在清理过程中触发信号
    if (!m_grid.empty()) {
        for (int i = 0; i < (int)m_grid.size(); i++) {
            for (int j = 0; j < (int)m_grid[i].size(); j++) {
                CellButton* btn = m_grid[i][j];
                if (btn) {
                    // 断开信号连接
                    disconnect(btn, &CellButton::leftClicked, this, &MineField::onCellLeftClick);
                    disconnect(btn, &CellButton::rightClicked, this, &MineField::onCellRightClick);
                    // 标记为删除，避免被父类重复释放
                    btn->setParent(nullptr);
                }
            }
        }
    }

    // 清理布局中的所有项目（但不删除 widget，因为马上要手动删除）
    while (m_layout && m_layout->count() > 0) {
        QLayoutItem* item = m_layout->takeAt(0);
        if (item && item->widget()) {
            // 不在这里 delete，后面统一删除
        }
        delete item;
    }

    // 手动删除所有旧的按钮
    if (!m_grid.empty()) {
        for (auto& row : m_grid) {
            for (auto* btn : row) {
                if (btn) {
                    delete btn;  // 直接删除
                }
            }
        }
        m_grid.clear();
    }

    // 清空所有容器
    m_isMine.clear();
    m_adjacentMines.clear();
    m_revealed.clear();
    m_flagged.clear();

    m_rows = rows;
    m_cols = cols;
    m_mineCount = mineCount;
    m_revealedCount = 0;
    m_flagCount = 0;
    m_gameOver = false;
    m_win = false;
    m_firstMove = true;

    // 重新初始化容器
    m_grid.resize(m_rows, std::vector<CellButton*>(m_cols, nullptr));
    m_isMine.resize(m_rows, std::vector<bool>(m_cols, false));
    m_adjacentMines.resize(m_rows, std::vector<int>(m_cols, 0));
    m_revealed.resize(m_rows, std::vector<bool>(m_cols, false));
    m_flagged.resize(m_rows, std::vector<bool>(m_cols, false));

    // 重新设置布局属性
    if (m_layout) {
        delete m_layout;
    }
    m_layout = new QGridLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);

    // 调整 widget 大小
    int buttonSize = 32;
    setFixedSize(cols * buttonSize, rows * buttonSize);

    // 创建新按钮
    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            CellButton *btn = new CellButton(i, j, this);
            connect(btn, &CellButton::leftClicked, this, &MineField::onCellLeftClick);
            connect(btn, &CellButton::rightClicked, this, &MineField::onCellRightClick);
            btn->setFixedSize(buttonSize, buttonSize);
            m_layout->addWidget(btn, i, j);
            m_grid[i][j] = btn;
        }
    }

    qDebug() << "=== newGame END ===";
    emit mineCountChanged(m_mineCount);
}

void MineField::initGrid()
{
    m_grid.resize(m_rows, std::vector<CellButton*>(m_cols, nullptr));
    m_isMine.resize(m_rows, std::vector<bool>(m_cols, false));
    m_adjacentMines.resize(m_rows, std::vector<int>(m_cols, 0));
    m_revealed.resize(m_rows, std::vector<bool>(m_cols, false));
    m_flagged.resize(m_rows, std::vector<bool>(m_cols, false));
    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            CellButton *btn = new CellButton(i, j, this);
            connect(btn, &CellButton::leftClicked, this, &MineField::onCellLeftClick);
            connect(btn, &CellButton::rightClicked, this, &MineField::onCellRightClick);
            btn->setFixedSize(32, 32);
            m_layout->addWidget(btn, i, j);
            m_grid[i][j] = btn;
        }
    }
}

void MineField::placeMines(int firstX, int firstY)
{
    QRandomGenerator *gen = QRandomGenerator::global();
    int placed = 0;
    while (placed < m_mineCount) {
        int x = gen->bounded(m_rows);
        int y = gen->bounded(m_cols);
        bool isFirstCell = (x == firstX && y == firstY);
        bool isAdjacent = (qAbs(x - firstX) <= 1 && qAbs(y - firstY) <= 1);

        if (!m_isMine[x][y] && !isFirstCell && !isAdjacent) {
            m_isMine[x][y] = true;
            placed++;
        }
    }
}

void MineField::calculateNumbers()
{
    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            if (m_isMine[i][j]) {
                m_adjacentMines[i][j] = -1;
                continue;
            }
            int count = 0;
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    if (di == 0 && dj == 0) continue;
                    int ni = i + di;
                    int nj = j + dj;
                    if (ni >= 0 && ni < m_rows && nj >= 0 && nj < m_cols) {
                        if (m_isMine[ni][nj]) count++;
                    }
                }
            }
            m_adjacentMines[i][j] = count;
        }
    }
}

void MineField::resetAllButtons()
{
    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            if (m_grid[i][j]) {
                m_grid[i][j]->reset();
            }
        }
    }
}

void MineField::revealCell(int x, int y)
{
    // 边界检查
    if (x < 0 || x >= m_rows || y < 0 || y >= m_cols) return;
    if (m_gameOver || m_win) return;
    if (m_revealed[x][y]) return;
    if (m_flagged[x][y]) return;

    // 第一步处理：如果是第一步，先布雷
    if (m_firstMove) {
        // 先重置所有数据
        for (int i = 0; i < m_rows; i++) {
            for (int j = 0; j < m_cols; j++) {
                m_isMine[i][j] = false;
                m_adjacentMines[i][j] = 0;
            }
        }
        // 布雷，避开第一次点击的位置
        placeMines(x, y);
        calculateNumbers();
        m_firstMove = false;
    }

    // 使用队列代替递归，避免栈溢出
    std::queue<std::pair<int, int>> toReveal;
    toReveal.push({x, y});

    while (!toReveal.empty()) {
        auto [cx, cy] = toReveal.front();
        toReveal.pop();

        // 边界检查
        if (cx < 0 || cx >= m_rows || cy < 0 || cy >= m_cols) continue;
        if (m_revealed[cx][cy]) continue;
        if (m_flagged[cx][cy]) continue;
        if (m_isMine[cx][cy]) {
            // 如果是地雷，单独处理
            playLoseSound();
            m_revealed[cx][cy] = true;
            m_revealedCount++;
            CellButton *btn = m_grid[cx][cy];
            btn->setText("💣");
            btn->setStyleSheet("background-color: red;");
            btn->setEnabled(false);
            m_gameOver = true;
            revealAllMines();
            emit gameOver(false);
            return;
        }

        // 翻开当前格子
        m_revealed[cx][cy] = true;
        m_revealedCount++;

        CellButton *btn = m_grid[cx][cy];
        int num = m_adjacentMines[cx][cy];

        if (num > 0) {
            btn->setText(QString::number(num));
            QString color;
            switch(num) {
            case 1: color = "blue"; break;
            case 2: color = "green"; break;
            case 3: color = "red"; break;
            default: color = "purple"; break;
            }
            btn->setStyleSheet(QString("color: %1; font-weight: bold; background-color: #cccccc;").arg(color));
            btn->setEnabled(false);
        } else {
            btn->setText("");
            btn->setStyleSheet("background-color: #cccccc;");
            btn->setEnabled(false);

            // 将周围未翻开、未标记的格子加入队列
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    if (di == 0 && dj == 0) continue;
                    int ni = cx + di;
                    int nj = cy + dj;
                    if (ni >= 0 && ni < m_rows && nj >= 0 && nj < m_cols) {
                        if (!m_revealed[ni][nj] && !m_flagged[ni][nj]) {
                            toReveal.push({ni, nj});
                        }
                    }
                }
            }
        }
    }

    checkWin();
}

void MineField::revealAllMines()
{
    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            if (m_isMine[i][j] && !m_revealed[i][j]) {
                m_grid[i][j]->setText("💣");
                m_grid[i][j]->setEnabled(false);
            }
        }
    }
}

void MineField::checkWin()
{
    int totalNonMines = m_rows * m_cols - m_mineCount;
    if (m_revealedCount == totalNonMines && !m_gameOver) {
        m_gameOver = true;
        m_win = true;
        playWinSound();

        // 标记所有地雷位置为旗子（胜利时）
        for (int i = 0; i < m_rows; i++) {
            for (int j = 0; j < m_cols; j++) {
                if (m_isMine[i][j]) {
                    m_grid[i][j]->setText("🥕");
                }
            }
        }

        emit gameOver(true);
    }
}

void MineField::onCellLeftClick(int x, int y)
{
    if (!m_gameOver && !m_win) {
        revealCell(x, y);
    }
}

void MineField::onCellRightClick(int x, int y)
{
    if (m_gameOver || m_win) return;
    if (m_revealed[x][y]) return;

    CellButton *btn = m_grid[x][y];

    if (!m_flagged[x][y]) {
        m_flagged[x][y] = true;
        btn->setText("🥕");
        m_flagCount++;
        emit mineCountChanged(m_mineCount - m_flagCount);
    } else {
        m_flagged[x][y] = false;
        btn->setText("");
        m_flagCount--;
        emit mineCountChanged(m_mineCount - m_flagCount);
    }
}
