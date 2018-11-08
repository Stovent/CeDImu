#include "GamePanel.hpp"

GamePanel::GamePanel(MainFrame* parent, CeDImu* appp) : wxPanel(parent)
{
    mainFrame = parent;
    app = appp;
}
