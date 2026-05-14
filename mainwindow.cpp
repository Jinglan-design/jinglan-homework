#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include<QInputDialog>
#include<QComboBox>
#include<QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_mineField(nullptr)
    , m_mineCountLabel(nullptr)
    , m_newGameBtn(nullptr)
{
    MineField::initSoundEffects();
    setupUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    QHBoxLayout *topLayout = new QHBoxLayout();
    m_mineCountLabel = new QLabel("剩余雷数: 10", this);
    m_mineCountLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 5px;");
    m_mineCountLabel->setFixedWidth(120);

    m_soundCheckBox = new QCheckBox("🔊 音效", this);
    m_soundCheckBox->setChecked(true); 
    m_soundCheckBox->setFixedWidth(80);


    m_difficultyCombo = new QComboBox(this);
    m_difficultyCombo->addItem("🌱 初级 (9x9, 10雷)");
    m_difficultyCombo->addItem("⚡ 中级 (16x16, 40雷)");
    m_difficultyCombo->addItem("💀 高级 (16x30, 99雷)");
    m_difficultyCombo->addItem("🎲 自定义...");
    m_difficultyCombo->setMinimumWidth(160);

    m_customBtn = new QPushButton("自定义", this);
    m_customBtn->setFixedSize(70, 28);

    m_newGameBtn = new QPushButton("新游戏", this);
    m_newGameBtn->setFixedSize(70, 28);

    topLayout->addWidget(m_mineCountLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_difficultyCombo);
    topLayout->addSpacing(8);
    topLayout->addWidget(m_customBtn);
    topLayout->addSpacing(8);
    topLayout->addWidget(m_newGameBtn);

    QWidget *gameContainer = new QWidget(this);
    QHBoxLayout *containerLayout = new QHBoxLayout(gameContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    m_mineField = new MineField(gameContainer);
    containerLayout->addWidget(m_mineField, 0, Qt::AlignCenter);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(gameContainer, 1);

    connect(m_newGameBtn, &QPushButton::clicked, this, &MainWindow::onNewGame);
    connect(m_customBtn, &QPushButton::clicked, this, &MainWindow::onCustomGame);
    connect(m_difficultyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDifficultyChanged);
    connect(m_mineField, &MineField::gameOver, this, &MainWindow::onGameOver);
    connect(m_mineField, &MineField::mineCountChanged, this, &MainWindow::onMineCountChanged);
    connect(m_soundCheckBox, &QCheckBox::toggled, this, &MainWindow::onSoundToggled);

    setWindowTitle("扫雷游戏");
    setMinimumSize(550, 400);
}

void MainWindow::onSoundToggled(bool checked)
{
    MineField::setSoundEnabled(checked);
}

void MainWindow::onNewGame()
{
    if (m_mineField) {
        int currentIndex = m_difficultyCombo->currentIndex();
        onDifficultyChanged(currentIndex);
    }
}

void MainWindow::startGame(int rows, int cols, int mineCount)
{
    if (m_mineField) {
        int buttonSize = 32;
        int margin = 40;
        int topBarHeight = 100; 

        int width = cols * buttonSize + margin;
        int height = rows * buttonSize + topBarHeight;

        width = qMax(width, 500);
        height = qMax(height, 450);

        width = qMin(width, 1400);
        height = qMin(height, 900);

        resize(width, height);

        if (centralWidget()) {
            centralWidget()->setMinimumSize(width, height);
        }

        m_mineField->newGame(rows, cols, mineCount);

        QApplication::processEvents();
    }
}

void MainWindow::onDifficultyChanged(int index)
{
    switch (index) {
    case 0:
        startGame(9, 9, 10);
        break;
    case 1:
        startGame(16, 16, 40);
        break;
    case 2:
        startGame(16, 30, 99);
        break;
    case 3:
        onCustomGame();
        break;
    }
}

void MainWindow::onCustomGame()
{
    bool ok1, ok2, ok3;

    int rows = QInputDialog::getInt(this, "自定义游戏", "行数 (8-30):", 16, 8, 30, 1, &ok1);
    int cols = QInputDialog::getInt(this, "自定义游戏", "列数 (8-40):", 16, 8, 40, 1, &ok2);
    int maxMines = (rows * cols) - 1;
    int defaultMines = qMin(rows * cols / 5, maxMines);
    int mineCount = QInputDialog::getInt(this, "自定义游戏", QString("雷数 (1-%1):").arg(maxMines), defaultMines, 1, maxMines, 1, &ok3);
    if (ok1 && ok2 && ok3) {
        m_difficultyCombo->setItemText(3, QString("🎲 自定义 (%1x%2, %3雷)").arg(rows).arg(cols).arg(mineCount));
        startGame(rows, cols, mineCount);
    } else {
        m_difficultyCombo->setCurrentIndex(0);
    }
}
void MainWindow::onGameOver(bool win)
{
    if (win) {
        QMessageBox::information(this, "胜利", "🎉 蒸蚌！！！ 🎉");
    } else {
        QMessageBox::information(this, "游戏结束", "💣 踩到地雷了！ 💣");
    }
}

void MainWindow::onMineCountChanged(int count)
{
    m_mineCountLabel->setText(QString("剩余雷数: %1").arg(count));
}
