#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <set>
#include <map>


class Sheet : public SheetInterface {
public:
    ~Sheet();
    bool CheckCell(Position pos) const;
    Cell* CreateCell(Position pos);
    void SetCell(Position pos, std::string text) override;

    const Cell* GetCell(Position pos) const override;
    Cell* GetCell(Position pos) override;
    void ClearCell(Position pos) override;
    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    std::unordered_map<Position, std::unique_ptr<Cell>, HashPosition> table_matrix;
    std::map<int, int> work_space_row_long;
    std::map<int, int> work_space_col_long;
    
	// Можете дополнить ваш класс нужными полями и методами
};