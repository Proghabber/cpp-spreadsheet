#include "cell.h"


#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <unordered_set>

Cell::Cell(SheetInterface& table):my_table_(table), impl_ (std::make_unique<EmptyImpl>())
{
}

Cell::~Cell()
{
}

void Cell::Set(std::string text) {
        if(text.size() == 0){
            impl_ = std::make_unique<EmptyImpl>();
        } else if (text.at(0) == '=' && text.size() > 1){
            auto impl_exam = std::make_unique<FormulaImpl>(text.substr(1), my_table_);
            std::unordered_set<Cell*> visit;
            std::vector<Position> path = impl_exam.get()->GetReferencedCells();
            if (FindRepid(visit, path)){
                throw CircularDependencyException("Circular Formula");
            } else {
                InvalidateDependet();
                impl_ = move(impl_exam);
            }
        } else if (text.size() > 0 && text.at(0) != '='){
            impl_ =  std::make_unique<TextImpl>(text);
            
        } 
        InvalidateDependet();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    if (cash_.has_value()){
        return cash_.value();
    }
    cash_ = impl_->GetValue();
    return cash_.value();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_.get()->GetReferencedCells();
}

void Cell::InvalidateDependet(){
    cash_.reset();
    for (auto from: from_cells_){
        from->InvalidatInvalidCash();
    }
}

void Cell::InvalidatInvalidCash(){
    cash_.reset();
}

void Cell::SetDepended(Cell *cell){
    from_cells_.insert(cell);
}

bool Cell::FindRepid(std::unordered_set<Cell*>& visit, std::vector<Position> paths){
    next_cells_.clear();
        
    for(auto pos: paths){
        Cell* next = static_cast<Cell*>(my_table_.GetCell(pos));
        if (next){
            next_cells_.insert(next);
        }   
    }

    if (visit.find(this) != visit.end()){ //если уже посетили
        return true;
    }
    visit.insert(this);
    for (Cell* next: next_cells_){
        if (next->FindRepid(visit, next->GetReferencedCells())){
            return true;
        }   
    }
    return false;
}

EmptyImpl::EmptyImpl(){
}

Impl::Value EmptyImpl::GetValue() const {
    return "";
}

std::string EmptyImpl::GetText() const {
    return "";
}

std::vector<Position> EmptyImpl::GetReferencedCells() const {
    std::vector<Position> empty(0);
    return empty;
}

TextImpl::TextImpl(std::string val): value(val)
{
}

Impl::Value TextImpl::GetValue() const {
    if (value.size() > 0 && value.at(0) =='\''){
        return std::string{value.begin() + 1, value.end()};
    }
    return value;
}

std::string TextImpl::GetText() const {
    return value;
}

std::vector<Position> TextImpl::GetReferencedCells() const {
    std::vector<Position> empty(0);
    return empty;
}


std::vector<Position> FormulaImpl::GetReferencedCells() const{
    return value->GetReferencedCells();
}

FormulaImpl::FormulaImpl(std::string val, SheetInterface&  table):my_table_(table), value(ParseFormula(val))
{
}

Impl::Value FormulaImpl::GetValue() const {
    if (std::holds_alternative<FormulaError>(value.get()->Evaluate(my_table_))){
        return std::get<FormulaError>(value.get()->Evaluate(my_table_));
    } 
    return std::get<double>(value.get()->Evaluate(my_table_));
     
}

std::string FormulaImpl::GetText() const {
    std::string text_rormula = "=";
    text_rormula += value->GetExpression();
    return text_rormula;
}

