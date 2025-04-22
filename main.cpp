#include "imconfig.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl3_loader.h"
#include "imgui_internal.h"
#include "imstb_rectpack.h"
#include "imstb_textedit.h"
#include "imstb_truetype.h"
#include <iostream>
#include <string>
#include "libraries/include/glad/glad.h"
#include "libraries/include/GLFW/glfw3.h"
#include "libraries/include/KHR/khrplatform.h"
#include <sqlite3.h>
#include "json.hpp"
#include <vector>
#include <algorithm>
#include <type_traits>
#include <filesystem>
#include <fstream>
#include <unordered_map>


using namespace std;

class General {
public:
    void setCode(int value) { code = value; }
    int getCode() const { return code; }

    void setName(string value) { name = value; }
    string getName() { return name; }

    General(int p_code, string p_name) : code(p_code), name(p_name) {}
    General() : code(0), name("") {}
private:
    string name;
    int code;
};

class Material : public General {
public:

    Material(int p_code, string p_name, int p_costPerGramm) : General(p_code, p_name), costPerGramm(p_costPerGramm) {}
    Material() : General(0, ""), costPerGramm(0) {}

    void setCostPerGramm(int value) { costPerGramm = value; }
    int getCostPerGramm() { return costPerGramm; }

private:
    int costPerGramm;
};

class Izdeliya : public General {
public:
    Izdeliya(Material p_material, int p_code, string p_name, string p_type, int p_weight, int p_cost) :
        General(p_code, p_name), material(p_material),
        type(p_type), weight(p_weight), cost(p_cost) {}
    Izdeliya() : General(0, ""), material(Material()), type(""), weight(0), cost(0) {}
    void setMaterial(Material value) { material = value; };
    Material getMaterial() { return material; };

    void setType(string value) { type = value; };
    string getType() { return type; };

    void setWeight(int value) { weight = value; };
    int getWeight() { return weight; };

    void setCost(int value) { cost = value; };
    int getCost() { return cost; };
private:
    Material material;
    string type;
    int weight;
    int cost;
};

class Prodazhi : public General {
public:
    Prodazhi(Izdeliya p_izdeliya, int p_code, string p_date, string p_name, string p_surname, string p_lastname) :
        General(p_code, p_name), izdeliya(p_izdeliya), date(p_date), surname(p_surname), lastname(p_lastname) {}

    Prodazhi() : General(0, ""), izdeliya(Izdeliya()), date(""), surname(""), lastname("") {}

    void setIzdeliya(Izdeliya value) { izdeliya = value; };
    Izdeliya getIzdeliya() { return izdeliya; };

    void setDate(string value) { date = value; };
    string getDate() { return date; };

    void setSurname(string value) { surname = value; };
    string getSurname() { return surname; };

    void setLastname(string value) { lastname = value; };
    string getLastname() { return lastname; };


    string getFIO() { return getSurname() + " " + getName() + " " + getLastname(); };
private:
    Izdeliya izdeliya;
    string date;
    string surname;
    string lastname;
};

template <typename T>
class GeneralList {
public:
    GeneralList() = default;
    void clear() { list.clear(); }
    T findByCode(int code) {
        for (const auto& item : list) {
            if (item.getCode() == code) return item;
        }
        return T();
    }

    vector<int> getCodes() {
        vector<int> codes;
        for (T item : list) {
            codes.push_back(item.getCode());
        }
        return codes;
    }

    int getNewCode() {
        vector<int> codes = this->getCodes();
        if (!codes.empty()) return *max_element(codes.begin(), codes.end());
        else return 1;
    }

    vector<T> getItems() { return list; };

    void appendItem(const T& item) {
        list.push_back(item);
    };
    void removeItem(const T& item) {
        if constexpr (is_same<T, int>::value) {
            auto it = remove(list.begin(), list.end(), this->findByCode(item));
            list.erase(it, list.end());
        }
        else if constexpr (is_same<T, General>::value) {
            auto it = remove(list.begin(), list.end(), item);
            list.erase(it, list.end());
        }

    };
    GeneralList(vector<T> p_list) : list(p_list) {}
private:
    vector<T> list;
};


class MaterialList : public GeneralList<Material> {
public:
    void appendItem(Material item) {
        GeneralList<Material>::appendItem(item);
    }
    Material createItem(int code, string name, int cost) {

        if (findByCode(code).getCode() != 0) {
            cout << "Материал с кодом " << code << " уже существует\n";
            return Material();
        }

        Material mat(code, name, cost);
        this->appendItem(mat);
        return mat;
    }
    Material newItem(string name, int cost) {
        Material mat(this->getNewCode(), name, cost);
        this->appendItem(mat);
        return mat;
    }
};


class IzdeliyaList : public GeneralList<Izdeliya> {
public:
    void appendItem(Izdeliya item) {
        GeneralList<Izdeliya>::appendItem(item);
    }
    Izdeliya createItem(int code, string name, string type, int weight, int cost, Material material) {
        if (findByCode(code).getCode() != 0) {
            cout << "Изделие с кодом " << code << " уже существует\n";
            return Izdeliya();
        }
        Izdeliya izd(material, code, name, type, weight, cost);
        this->appendItem(izd);
        return izd;
    }
    Izdeliya newItem(string name, string type, int weight, int cost, Material material) {
        Izdeliya izd(material, this->getNewCode(), name, type, weight, cost);
        this->appendItem(izd);
        return izd;
    }
};


class ProdazhiList : public GeneralList<Prodazhi> {
public:
    void appendItem(Prodazhi item) {
        GeneralList<Prodazhi>::appendItem(item);
    }
    Prodazhi createItem(int code, string date, string name, string surname, string lastname, Izdeliya izdeliya) {
        if (findByCode(code).getCode() != 0) {
            cout << "Продажа с кодом " << code << " уже существует\n";
            return Prodazhi();
        }
        Prodazhi prd(izdeliya, code, date, name, surname, lastname);
        this->appendItem(prd);
        return prd;

    }
    Prodazhi newItem(string date, string name, string surname, string lastname, Izdeliya izdeliya) {
        Prodazhi prd(izdeliya, this->getNewCode(), name, name, surname, lastname);
        this->appendItem(prd);
        return prd;
    }
};


class Spisok {
public:
    void clear() {
        materialList.clear();
        izdeliyaList.clear();
        prodazhiList.clear();
    }



    void createMaterial(int code, string name, int cost) { materialList.createItem(code, name, cost); }
    void newMaterial(string name, int cost) { materialList.newItem(name, cost); }
    void removeMaterial(Material value) {
        materialList.removeItem(value);
        for (Izdeliya item : izdeliyaList.getItems()) {
            if (item.getMaterial().getCode() == value.getCode()) {
                item.setMaterial(Material());
            }
        }
    }
    Material getMaterial(int code) { return materialList.findByCode(code); }
    vector<Material> getMaterialList() { return materialList.getItems(); }
    vector<int> getMaterialCodes() { return materialList.getCodes(); }
    int getMaterialNewCode() { return materialList.getNewCode(); }


    void createIzdeliya(int code, string name, string type, int weight, int cost, Material material) { izdeliyaList.createItem(code, name, type, weight, cost, material); }
    void newIzdeliya(string name, string type, int weight, int cost, Material material) { izdeliyaList.newItem(name, type, weight, cost, material); }
    void removeIzdeliya(Izdeliya value) {
        izdeliyaList.removeItem(value);
        for (Prodazhi item : prodazhiList.getItems()) {
            if (item.getIzdeliya().getCode() == value.getCode()) {
                item.setIzdeliya(Izdeliya());
            }
        }
    }
    Izdeliya getIzdeliya(int code) { return izdeliyaList.findByCode(code); }
    vector<Izdeliya> getIzdeliyaList() { return izdeliyaList.getItems(); }
    vector<int> getIzdeliyaCodes() { return izdeliyaList.getCodes(); }
    int getIzdeliyaNewCode() { return izdeliyaList.getNewCode(); }




    void createProdazhi(int code, string date, string name, string surname, string lastname, Izdeliya izdeliya) { prodazhiList.createItem(code, date, name, surname, lastname, izdeliya); }
    void newProdazhi(string date, string name, string surname, string lastname, Izdeliya izdeliya) { prodazhiList.newItem(date, name, surname, lastname, izdeliya); }
    void removeProdazhi(Prodazhi value) {
        prodazhiList.removeItem(value);
    }
    Prodazhi getProdazhi(int code) { return prodazhiList.findByCode(code); }
    vector<Prodazhi> getProdazhiList() { return prodazhiList.getItems(); }
    vector<int> getProdazhiCodes() { return prodazhiList.getCodes(); }
    int getProdazhiNewCode() { return prodazhiList.getNewCode(); }

private:
    MaterialList materialList;
    IzdeliyaList izdeliyaList;
    ProdazhiList prodazhiList;

};


class Data {
public:
    Data(Spisok& p_spisok, string p_inp, string p_out) : spisok(p_spisok), inp(p_inp), out(p_out) {}
    Spisok& getSpisok() { return spisok; }


    void setInp(string value) { inp = value; }
    string getInp() { return inp; }

    void setOut(string value) { out = value; }
    string getOut() { return out; }

    void readFile(string filename) {
        setInp(filename);
        read();
    }

    void writeFile(string filename) {
        setOut(filename);
        write();
    }

    void read() {}
    void write() {}
private:
    string inp;
    string out;
    Spisok& spisok;
};

class Datajson : public Data {
public:
    Datajson(Spisok& p_spisok, string p_inp, string p_out) : Data(p_spisok, p_inp, p_out) {}
    void read() {
        ifstream jsonInp(getInp());
        if (!jsonInp.is_open()) {
            cout << "Ошибка: не удалось открыть файл " << getInp() << " для чтения." << endl;
            return;
        }

        nlohmann::json datajson;
        jsonInp >> datajson;

        for (const auto& item : datajson["material"]) {
            int code = 0; string name = ""; int cost = 0;
            code = item["code"];
            name = item["name"];
            cost = item["cost"];
            getSpisok().createMaterial(code, name, cost);
        }

        for (const auto& item : datajson["izdeliya"]) {
            int code = 0; string name = ""; string type = ""; int weight = 0; int cost = 0; Material material = Material();
            code = item["code"];
            name = item["name"];
            type = item["type"];
            weight = item["weight"];
            cost = item["cost"];
            material = getSpisok().getMaterial(item["material"]);
            getSpisok().createIzdeliya(code, name, type, weight, cost, material);
        }
        for (const auto& item : datajson["prodazhi"]) {
            int code = 0; string date = ""; string name = ""; string surname = ""; string lastname = ""; Izdeliya izdeliya = Izdeliya();
            code = item["code"];
            date = item["date"];
            name = item["name"];
            surname = item["surname"];
            lastname = item["lastname"];
            izdeliya = getSpisok().getIzdeliya(item["izdeliya"]);
            getSpisok().createProdazhi(code, date, name, surname, lastname, izdeliya);
        }
    }
    void write() {
        nlohmann::json datajson;
        datajson["material"] = nlohmann::json::array();
        datajson["izdeliya"] = nlohmann::json::array();
        datajson["prodazhi"] = nlohmann::json::array();

        for (Material item : getSpisok().getMaterialList()) {
            nlohmann::json materialJson;
            materialJson["code"] = item.getCode();
            materialJson["name"] = item.getName();
            materialJson["cost"] = item.getCostPerGramm();
            datajson["material"].push_back(materialJson);
        }

        for (Izdeliya item : getSpisok().getIzdeliyaList()) {
            nlohmann::json izdeliyaJson;
            izdeliyaJson["code"] = item.getCode();
            izdeliyaJson["name"] = item.getName();
            izdeliyaJson["type"] = item.getType();
            izdeliyaJson["weight"] = item.getWeight();
            izdeliyaJson["cost"] = item.getCost();
            izdeliyaJson["material"] = item.getMaterial().getCode();
            datajson["izdeliya"].push_back(izdeliyaJson);
        }

        for (Prodazhi item : getSpisok().getProdazhiList()) {
            nlohmann::json prodazhiJson;
            prodazhiJson["code"] = item.getCode();
            prodazhiJson["date"] = item.getDate();
            prodazhiJson["name"] = item.getName();
            prodazhiJson["surname"] = item.getSurname();
            prodazhiJson["lastname"] = item.getLastname();
            prodazhiJson["izdeliya"] = item.getIzdeliya().getCode();
            datajson["prodazhi"].push_back(prodazhiJson);
        }

        ofstream jsonOut(getOut());
        if (!jsonOut.is_open()) {
            cout << "Ошибка: не удалось открыть файл " << getOut() << " для записи." << endl;
            return;
        }
        jsonOut << setw(4) << datajson;
        jsonOut.close();
        cout << "Файл " << getOut() << " записан успешно." << endl;
    }
};

class Datasql : public Data {
public:
    Datasql(Spisok& p_spisok, string p_inp, string p_out) : Data(p_spisok, p_inp, p_out) {}
    void read() {
        sqlite3* db;
        sqlite3_stmt* stmt;
        if (sqlite3_open(getInp().c_str(), &db) != SQLITE_OK) {
            cerr << "Не удалось создать БД" << endl;
            return;
        }

        // Читаем таблицу material
        if (sqlite3_prepare_v2(db, "SELECT code, name, costpergramm FROM material", -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                getSpisok().createMaterial(
                    sqlite3_column_int(stmt, 0),
                    reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
                    sqlite3_column_int(stmt, 2)
                );
            }
            sqlite3_finalize(stmt);
        }

        // Читаем таблицу izdeliya
        if (sqlite3_prepare_v2(db, "SELECT code, name, type, weight, cost, material FROM izdeliya", -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                getSpisok().createIzdeliya(
                    sqlite3_column_int(stmt, 0),
                    reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
                    reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
                    sqlite3_column_int(stmt, 3),
                    sqlite3_column_int(stmt, 4),
                    getSpisok().getMaterial(sqlite3_column_int(stmt, 5))
                );
            }
            sqlite3_finalize(stmt);
        }

        // Читаем таблицу prodazhi
        if (sqlite3_prepare_v2(db, "SELECT code, date, name, surname, lastname, izdeliya FROM prodazhi", -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                getSpisok().createProdazhi(
                    sqlite3_column_int(stmt, 0),
                    reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
                    reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
                    reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
                    reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)),
                    getSpisok().getIzdeliya(sqlite3_column_int(stmt, 5))
                );
            }
            sqlite3_finalize(stmt);
        }

        sqlite3_close(db);
    }

    void write() {
        sqlite3* db;
        char* errMsg = nullptr;
        if (sqlite3_open(getOut().c_str(), &db) != SQLITE_OK) {
            cerr << "БД не открылась" << endl;
            return;
        }

        string emptydb = "PRAGMA foreign_keys=ON;"
            "CREATE TABLE material (code INTEGER PRIMARY KEY, name TEXT, costpergramm INTEGER);"
            "CREATE TABLE izdeliya (code INTEGER PRIMARY KEY, name TEXT, type TEXT, weight INTEGER, cost INTEGER, material INTEGER REFERENCES material(code) ON UPDATE CASCADE ON DELETE SET NULL);"
            "CREATE TABLE prodazhi (code INTEGER PRIMARY KEY, date TEXT, name TEXT, surname TEXT, lastname TEXT, izdeliya INTEGER REFERENCES izdeliya(code) ON UPDATE CASCADE ON DELETE SET NULL);";
        if (sqlite3_exec(db, emptydb.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            cerr << "Не получилось создать таблицы: " << errMsg << endl;
            sqlite3_free(errMsg);
        }

        for (Material m : getSpisok().getMaterialList()) {
            string query = "INSERT INTO material (code, name, costpergramm) VALUES (" +
                to_string(m.getCode()) + ", '" + m.getName() + "', " + to_string(m.getCostPerGramm()) + ");";
            sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
        }

        for (Izdeliya i : getSpisok().getIzdeliyaList()) {
            string query = "INSERT INTO izdeliya (code, name, type, weight, cost, material) VALUES (" +
                to_string(i.getCode()) + ", '" + i.getName() + "', '" + i.getType() + "', " +
                to_string(i.getWeight()) + ", " + to_string(i.getCost()) + ", " +
                to_string(i.getMaterial().getCode()) + ");";
            sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
        }

        for (Prodazhi p : getSpisok().getProdazhiList()) {
            string query = "INSERT INTO prodazhi (code, date, name, surname, lastname, izdeliya) VALUES (" +
                to_string(p.getCode()) + ", '" + p.getDate() + "', '" + p.getName() + "', '" +
                p.getSurname() + "', '" + p.getLastname() + "', " +
                to_string(p.getIzdeliya().getCode()) + ");";
            sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
        }

        sqlite3_close(db);
        cout << "Файл " << getOut() << " записан успешно." << endl;
    }
};


//int main() {
//
//    setlocale(LC_ALL, "ru_RU.UTF-8");
//
//    Spisok spis1;
//    Spisok spis2;
//
//    Datajson djson1(spis1, "old_jewelry_data_corrected.json", "new_jewelry_data_corrected.json");
//    Datajson djson2(spis2, "old_jewelry_data_corrected.json", "new_jewelry_data_corrected.json");
//
//    Datasql dsql1(spis1, "new.sqlite", "new.sqlite");
//    Datasql dsql2(spis2, "new.sqlite", "new.sqlite");
//    djson1.read();
//    if (filesystem::exists(dsql1.getOut())) { filesystem::remove(dsql1.getOut()); }
//    dsql1.write();
//    dsql2.read();
//    djson2.write();
//
//
//    system("pause");
//    return 0;
//}

class RowCode {
public:
    void clear() {
        list.clear();
    }
    void appendRowCode(int row, int code) {
        list[row] = code;
    }
    void updateRow(int row) {
        unordered_map<int, int> new_list = {};
        for (const auto& para : list) {
            if (para.first > row) new_list[para.first - 1] = para.second;
            else new_list[para.first] = para.second;
        }
        list = move(new_list);
    }

    void removeRow(int row) {
        list.erase(row);
        updateRow(row);
    }

    void removeCode(int code) {
        for (auto it = list.begin(); it != list.end();) {
            if (it->second == code) it = list.erase(it);
            else ++it;
        }
    }

    const int getCode(int row) {
        auto it = list.find(row);
        if (it != list.end()) return it->second;
        return 0;
    }

    const vector<int> getCodes() {
        vector<int> codes;
        for (const auto& para : list) codes.push_back(para.second);
        return codes;
    }

    const int getRow(int code) {
        for (const auto& para : list) if (para.second == code) return para.first;
        return 0;
    }
private:
    unordered_map<int, int> list = {};
};

class SpisWidget {
public:
    SpisWidget(Spisok* p_spisok = nullptr) : spisok(p_spisok) {}

    Spisok* getSpisok() { return spisok; }
private:
    Spisok* spisok;
};

class dbTable : public RowCode {
protected:
    vector<string> headers;
    vector<vector<string>> data;

    void addRowInternal(const vector<string>& rowData) {
        if (rowData.size() == headers.size()) {
            data.push_back(rowData);
        }
    }

public:
    dbTable(const vector<string>& columnHeaders) : headers(columnHeaders) {}

    

    void display() {
        if (ImGui::BeginTable("Table", headers.size(), ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            // Заголовки
            for (const auto& header : headers) {
                ImGui::TableSetupColumn(header.c_str());
            }
            ImGui::TableHeadersRow();

            for (int row = 0; row < data.size(); ++row) {
                ImGui::TableNextRow();

                // Флаг для отслеживания клика по строке
                bool rowClicked = false;

                for (int col = 0; col < data[row].size(); ++col) {
                    ImGui::TableSetColumnIndex(col);

                    // Создаем уникальный идентификатор для каждой ячейки
                    string label = "##" + to_string(row) + "_" + to_string(col);

                    // Используем Selectable для кликабельной ячейки
                    if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_SpanAllColumns)) {
                        rowClicked = true; // Помечаем, что строка была кликнута
                    }

                    // Отображаем текст ячейки
                    ImGui::SameLine();
                    ImGui::TextUnformatted(data[row][col].c_str());
                }

                // Если строка была кликнута, выполняем действие
                if (rowClicked) {
                    int code = getCode(row); // Получаем код по строке
                    cout << "Вы кликнули на строку: " << row << ", код: " << code << endl;
                    // Здесь можно добавить логику для обработки клика
                }
            }
            ImGui::EndTable();
        }
    }
};

class MaterialTable : public dbTable {
public:
    MaterialTable() : dbTable({ "Название", "Цена за грамм" }) {}

    void addMaterial(int code, const string& name, int cost) {
        addRowInternal({ name, to_string(cost) });
        appendRowCode(data.size() - 1, code); // Связываем строку с кодом материала
    }
};

class IzdeliyaTable : public dbTable {
public:
    IzdeliyaTable() : dbTable({ "Название", "Тип", "Вес", "Цена", "Материал" }) {}

    void addIzdeliya(int code, const string& name, const string& type, int weight, int cost, const string& materialName) {
        addRowInternal({ name, type, to_string(weight), to_string(cost), materialName });
        appendRowCode(data.size() - 1, code); // Используем метод из RowCode
    }
};

class ProdazhiTable : public dbTable {
public:
    ProdazhiTable() : dbTable({ "Дата", "ФИО", "Изделие" }) {}

    void addProdazhi(int code, const string& date, const string& fio, const string& izdeliyaName) {
        addRowInternal({ date, fio, izdeliyaName });
        appendRowCode(data.size() - 1, code); // Используем метод из RowCode
    }
};

class Tab
{
public:
    Tab(const string& name) : tabName(name) {}

    void Draw()
    {
        if (ImGui::BeginTable("ClickableTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            // Заголовки столбцов
            ImGui::TableSetupColumn("Column 1");
            ImGui::TableSetupColumn("Column 2");
            ImGui::TableSetupColumn("Column 3");
            ImGui::TableHeadersRow();

            // Содержимое таблицы
            for (int row = 0; row < 5; row++) {
                ImGui::TableNextRow(); // Переход на следующую строку

                // Флаг для отслеживания клика по строке
                bool rowClicked = false;

                for (int col = 0; col < 3; col++) {
                    ImGui::TableSetColumnIndex(col); // Переход на конкретный столбец

                    // Создаем кликабельную ячейку с помощью Selectable
                    char label[32];
                    snprintf(label, IM_ARRAYSIZE(label), "Row %d, Col %d", row, col);

                    // Используем Selectable для кликабельной ячейки
                    if (ImGui::Selectable(label, false, ImGuiSelectableFlags_SpanAllColumns)) {
                        rowClicked = true; // Помечаем, что строка была кликнута
                    }
                }

                // Если строка была кликнута, выполняем действие
                if (rowClicked) {
                    ImGui::Text("You clicked row %d!", row);
                }
            }

            // Завершаем таблицу
            ImGui::EndTable();
        }
    }

private:
    string tabName; // Название вкладки
    int counter = 0; // Счётчик
    char textInput[128] = ""; // Поле ввода текста
    int comboSelection = 0; // Выбор в комбо-боксе
    const char* comboItems[3] = { "Option 1", "Option 2", "Option 3" }; // Элементы комбо-бокса
};



int main()
{   
    setlocale(LC_ALL, "ru_RU.UTF-8");
    glfwInit();
    // Указываем версию OpenGL 3.3 (Core Profile)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    

    // Создание окна GLFW
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui + GLFW + OpenGL3 Example", NULL, NULL);
    if (!window)
    {
        cout << "Не запистилось окно\n";
        glfwTerminate();
        return -1;
    }
    
    // Сделать контекст текущим
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Включить вертикальную синхронизацию
    gladLoadGL();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\bahnschrift.ttf", 20, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    Tab tab1("Tab 1");
    MaterialTable materialTable;
    materialTable.addMaterial(1, "Золото", 5000);
    materialTable.addMaterial(2, "Серебро", 300);

    IzdeliyaTable izdeliyaTable;
    izdeliyaTable.addIzdeliya(101, "Кольцо", "Украшение", 10, 10000, "Золото");
    izdeliyaTable.addIzdeliya(102, "Цепочка", "Украшение", 5, 5000, "Серебро");

    ProdazhiTable prodazhiTable;
    prodazhiTable.addProdazhi(1001, "2023-10-01", "Иванов Иван Иванович", "Кольцо");
    prodazhiTable.addProdazhi(1002, "2023-10-02", "Петров Петр Петрович", "Цепочка");


    bool showAboutPopup = false;
    // Основной цикл
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Full Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);


  

        // Окно "О программе"
        if (showAboutPopup) {
            ImGui::OpenPopup("О программе");
            if (ImGui::BeginPopupModal("О программе", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Программа для управления базой данных ювелирных изделий.");
                ImGui::Text("Версия 1.0");
                ImGui::Separator();
                if (ImGui::Button("Закрыть")) {
                    showAboutPopup = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }


        if (ImGui::BeginTabBar("MainTabBar"))
        {
            if (ImGui::BeginTabItem("Материалы")) {
                materialTable.display();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Изделия")) {
                izdeliyaTable.display();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Продажи")) {
                prodazhiTable.display();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();

        // Рендеринг
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
