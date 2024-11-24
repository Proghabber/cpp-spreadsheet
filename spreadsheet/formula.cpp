#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) try
        : ast_(ParseFormulaAST(expression)) {
    } catch (const std::exception& exc) {
        throw FormulaException("ARITHM!");
    }
         
    Value Evaluate(const SheetInterface& sheet) const override {
        Value rezult;
        try {
            rezult = ast_.Execute(sheet);
        } catch (const FormulaError::Category& exc) {
            if (exc == FormulaError::Category::Value){
                rezult = FormulaError::Category::Value;
            } else if (exc == FormulaError::Category::Arithmetic){
                rezult = FormulaError::Category::Arithmetic;
            } 
        }
        return rezult;
    }

    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const override{
        std::vector<Position> all_cells_in_formula;
        std::set<Position> cut_cells_in_formula;
        const std::forward_list<Position>& list =  ast_.GetCells();
        auto start = list.begin();
        auto end = list.end();
        while (start != end){
            if (cut_cells_in_formula.find(*start) == cut_cells_in_formula.end()){
                cut_cells_in_formula.insert(*start);
                all_cells_in_formula.push_back(*start);
            }
            start++;   
        }
        return all_cells_in_formula; //выполнено
    }
private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}