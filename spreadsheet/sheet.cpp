#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

bool Sheet::CheckCell(Position pos) const{
    if (table_matrix.count(pos)){
        return true;
    }
    return false;
}

Cell* Sheet::CreateCell(Position pos){
    //если такой позиции нет создаст ее пустой и вернет, если есть вернет ее 
    //бросит исключение если позиция не валидна
    bool rez = !pos.IsValid();
    if (rez){
       throw InvalidPositionException("invalid position");
    } 
    table_matrix.emplace(pos, std::make_unique<Cell>(*this));
    return table_matrix.at(pos).get();
}

void Sheet::SetCell(Position pos, std::string text) {
    Cell* cell_exam = CreateCell(pos);
    work_space_row_long[pos.row]++;
    work_space_col_long[pos.col]++;
    
    cell_exam->Set(text);
    for (auto pos: cell_exam->GetReferencedCells()){
        Cell* next = GetCell(pos);
        if (next){
            next->SetDepended(cell_exam);
        }    
    }   
}

const Cell* Sheet::GetCell(Position pos) const {
    bool rez = !pos.IsValid();
    if (rez){
       throw InvalidPositionException("invalid position");
    }else if (!CheckCell(pos)){
        return nullptr;
    }
    return table_matrix.at(pos).get();
}

Cell* Sheet::GetCell(Position pos) {
    bool rez = !pos.IsValid();
    if (rez){
       throw InvalidPositionException("invalid position");
    }else if (!CheckCell(pos)){
        return nullptr;
    }
    return table_matrix.at(pos).get();
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()){
        throw InvalidPositionException("invalid position");
    }
    if (!CheckCell(pos)){
        return;
    }
    table_matrix.at(pos) = nullptr;
    
    if (work_space_row_long.count(pos.row)){
        work_space_row_long[pos.row]--;
        if (work_space_row_long[pos.row] <= 0){
            auto key = work_space_row_long.find(pos.row);
            work_space_row_long.erase(key);
        }
    }

    if (work_space_col_long.count(pos.col)){
        work_space_col_long[pos.col]--;
        if (work_space_col_long[pos.col] <= 0){
            auto key = work_space_col_long.find(pos.col);
            work_space_col_long.erase(key);
        }
    }
}

Size Sheet::GetPrintableSize() const {
    int col = 0;
    int row = 0;
    if (!work_space_row_long.empty()){
        row = work_space_row_long.rbegin()->first + 1;
    }
    if (!work_space_col_long.empty()){
        col = work_space_col_long.rbegin()->first + 1;
    }
    return {row, col};
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; row++){ //перебираем колонны
        if (!work_space_row_long.count(row)){
            output<<'\t';
            output <<'\n';
            continue;
        }
        int count = work_space_row_long.at(row);
        for (int col = 0; col < GetPrintableSize().cols && count > 0; col++){ // идем по колоннам
            if (CheckCell({row, col})){
                auto val = GetCell({row, col})->GetValue();
                 if (std::holds_alternative<double>(val)){
                     output << std::get<double>(val);
                     if (GetCell({row, col})->GetText() != ""){
                        count--;
                    }
                 } else if (std::holds_alternative<std::string>(val)){
                     output << std::get<std::string>(val);
                     if (GetCell({row, col})->GetText() != ""){
                        count--;
                    }
                 } else if (std::holds_alternative<FormulaError>(val)){
                    output << std::get<FormulaError>(val);
                    if (GetCell({row, col})->GetText() != ""){
                        count--;
                    }
                 }  
            }
            if (count > 0){
                output<<'\t';
            }
        }
        output <<'\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; row++){ //перебираем колонны
        if (!work_space_row_long.count(row)){
            output<<'\t';
            output <<'\n';
            continue;
        }
        int count = work_space_row_long.at(row);
        for (int col = 0; col < GetPrintableSize().cols && count > 0; col++){ // идем по колоннам  
            if (CheckCell({row, col})){
                output<<GetCell({row, col})->GetText();
                 if (GetCell({row, col})->GetText() != ""){
                    count--;
                 } 
            }
            if (count > 0){
                output<<'\t';
             }
        }
        output <<'\n';
    }  
}


std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}