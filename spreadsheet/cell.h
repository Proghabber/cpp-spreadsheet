#pragma once


#include <unordered_set>
#include <optional>
#include <forward_list>
#include <sstream>
#include "common.h"
#include "formula.h"

class Cell;
class Impl {
public:
    using Value = std::variant<std::string, double, FormulaError>;

    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0; //вернет пустой список, если не формула
    
    virtual ~Impl() = default;
};

class EmptyImpl : public Impl {
public:    
    EmptyImpl();
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;// вернет пустой список 
};

class TextImpl : public Impl {
    std::string value;
public:    
    TextImpl(std::string val);
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;// вернет пустой список
};

class FormulaImpl : public Impl {
    SheetInterface& my_table_;// добавили, теперь можем зная адреса получить ссылки из яцеек 
    std::unique_ptr<FormulaInterface> value;  
    
public:    
    FormulaImpl(std::string val, SheetInterface&  table);
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override; //вернет все ячейки из формулы
};

class Cell : public CellInterface {
public:
    Cell(SheetInterface&  table);
    ~Cell();
    void Set(std::string text);
    void Clear();
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    void InvalidateDependet();
    void InvalidatInvalidCash();
    void SetDepended(Cell* cell);
    bool FindRepid(std::unordered_set<Cell*>& visit, std::vector<Position> paths); //поиск цикла

private:
    SheetInterface& my_table_;// чтобы не передавть в Set
    std::unique_ptr<Impl> impl_;
    std::unordered_set<Cell*> next_cells_; // по ним будем искать зависимости циклические для FormulaImpl если это будет необходимо отдельно
    std::unordered_set<Cell*> from_cells_;
    mutable std::optional<Cell::Value> cash_; // кешь будет вычислятся или обновлятся                                 
};