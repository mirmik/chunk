#include "guard/guard.h"
#include "apply.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::vector<std::string> read_lines(const fs::path &p);
int run_apply(const fs::path &patch);

static std::string yaml_path(const fs::path &p)
{
    return p.generic_string();
}

TEST_CASE("replace_py_block: main block with Cyrillic comments and multiple functions")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_replace_py_block_main";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "script.py";
    {
        std::ofstream out(f);
        out << "def util(x):\n"
               "    return x * 2\n"
               "\n"
               "if __name__ == \"__main__\":\n"
               "    # стартовая точка\n"
               "    def run():\n"
               "        # внутренняя функция\n"
               "        print(\"run\", util(10))\n"
               "\n"
               "    def helper():\n"
               "        # ещё одна функция\n"
               "        print(\"helper\")\n"
               "\n"
               "    run()\n";
    }

    fs::path patch = tmp / "patch_replace_main.yaml";
    {
        std::ofstream out(patch);
        out << "language: python\n"
               "operations:\n"
               "  - op: replace_py_block\n"
               "    path: \"" << yaml_path(f) << "\"\n"
               "    marker: |\n"
               "      if __name__ == \"__main__\":\n"
               "    payload: |\n"
               "      if __name__ == \"__main__\":\n"
               "          # новая точка входа\n"
               "          def run():\n"
               "              # новый run\n"
               "              print(\"RUN\", util(5))\n"
               "\n"
               "          def extra():\n"
               "              # дополнительная функция\n"
               "              print(\"EXTRA\")\n"
               "\n"
               "          extra()\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() >= 5);

    CHECK(lines[0] == "def util(x):");
    CHECK(lines[1] == "    return x * 2");

    int main_idx = -1;
    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        if (lines[i] == "if __name__ == \"__main__\":")
        {
            main_idx = static_cast<int>(i);
            break;
        }
    }
    REQUIRE(main_idx >= 0);
    REQUIRE(main_idx + 4 < static_cast<int>(lines.size()));

    CHECK(lines[main_idx + 1].find("новая точка входа") != std::string::npos);
    CHECK(lines[main_idx + 2].find("def run") != std::string::npos);
    CHECK(lines[main_idx + 3].find("новый run") != std::string::npos);
    CHECK(lines[main_idx + 4].find("RUN") != std::string::npos);
}

TEST_CASE("replace_py_block: method with docstring and nested functions")
{
    fs::path tmp = fs::temp_directory_path() /
                   "chunk_test_replace_py_block_method_docstring";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "service.py";
    {
        std::ofstream out(f);
        out << "class Service:\n"
               "    def handle(self, data):\n"
               "        \"\"\"Обработчик данных:\n"
               "        Принимает data и возвращает результат.\n"
               "        \"\"\"\n"
               "        # старый комментарий: не блок\n"
               "        if not data:\n"
               "            return None\n"
               "        return data * 2\n"
               "\n"
               "def unrelated():\n"
               "    return 42\n";
    }

    fs::path patch = tmp / "patch_replace_method.yaml";
    {
        std::ofstream out(patch);
        out << "language: python\n"
               "operations:\n"
               "  - op: replace_py_block\n"
               "    path: \"" << yaml_path(f) << "\"\n"
               "    marker: |\n"
               "      def handle(self, data):\n"
               "    payload: |\n"
               "      def handle(self, data):\n"
               "          # новый обработчик с кириллицей\n"
               "          if data is None:\n"
               "              return \"пусто\"\n"
               "\n"
               "          def inner(value):\n"
               "              # внутренняя функция с комментарием\n"
               "              return value * 10\n"
               "\n"
               "          return inner(data)\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(!lines.empty());
    CHECK(lines[0].find("class Service") != std::string::npos);

    bool has_old_comment = false;
    for (const auto &ln : lines)
    {
        if (ln.find("старый комментарий") != std::string::npos)
            has_old_comment = true;
    }
    CHECK(!has_old_comment);

    bool has_new_comment = false;
    bool has_inner = false;
    for (const auto &ln : lines)
    {
        if (ln.find("новый обработчик") != std::string::npos)
            has_new_comment = true;
        if (ln.find("def inner") != std::string::npos)
            has_inner = true;
    }
    CHECK(has_new_comment);
    CHECK(has_inner);
}

TEST_CASE("replace_py_block: chooses correct block using before context")
{
    fs::path tmp = fs::temp_directory_path() /
                   "chunk_test_replace_py_block_before_context";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "config_blocks.py";
    {
        std::ofstream out(f);
        out << "config = \"A\"\n"
               "if feature_enabled:\n"
               "    # блок для A\n"
               "    value = 1\n"
               "\n"
               "config = \"B\"\n"
               "if feature_enabled:\n"
               "    # блок для B\n"
               "    value = 2\n";
    }

    fs::path patch = tmp / "patch_replace_before.yaml";
    {
        std::ofstream out(patch);
        out << "language: python\n"
               "operations:\n"
               "  - op: replace_py_block\n"
               "    path: \"" << yaml_path(f) << "\"\n"
               "    marker: |\n"
               "      if feature_enabled:\n"
               "    before: |\n"
               "      config = \"B\"\n"
               "    payload: |\n"
               "      if feature_enabled:\n"
               "          # новая ветка для B\n"
               "          def run_b():\n"
               "              # ещё одна функция внутри блока\n"
               "              print(\"B\", value)\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);

    bool has_block_A = false;
    bool has_old_block_B_comment = false;
    bool has_new_block_B_comment = false;

    for (const auto &ln : lines)
    {
        if (ln.find("блок для A") != std::string::npos)
            has_block_A = true;
        if (ln.find("блок для B") != std::string::npos)
            has_old_block_B_comment = true;
        if (ln.find("новая ветка для B") != std::string::npos)
            has_new_block_B_comment = true;
    }

    CHECK(has_block_A);
    CHECK(!has_old_block_B_comment);
    CHECK(has_new_block_B_comment);
}

TEST_CASE("replace_py_block: options.indent = none keeps payload indentation as-is")
{
    fs::path tmp = fs::temp_directory_path() /
                   "chunk_test_replace_py_block_indent_none";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "indent.py";
    {
        std::ofstream out(f);
        out << "class Runner:\n"
               "    def main(self):\n"
               "        if enabled:\n"
               "            # старый блок\n"
               "            run_old()\n"
               "        self.done = True\n";
    }

    fs::path patch = tmp / "patch_replace_indent.yaml";
    {
        std::ofstream out(patch);
        out << "language: python\n"
               "operations:\n"
               "  - op: replace_py_block\n"
               "    path: \"" << yaml_path(f) << "\"\n"
               "    marker: |\n"
               "      if enabled:\n"
               "    options:\n"
               "      indent: none\n"
               "    payload: |\n"
               "      if enabled:\n"
               "          # новый блок без автоотступа\n"
               "          def local():\n"
               "              # локальная функция\n"
               "              print(\"локально\")\n"
               "          local()\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);

    int if_idx = -1;
    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        if (lines[i].find("if enabled:") != std::string::npos)
        {
            if_idx = static_cast<int>(i);
            break;
        }
    }
    REQUIRE(if_idx >= 0);
    CHECK(lines[if_idx] == "if enabled:");

    bool has_self_done = false;
    for (const auto &ln : lines)
    {
        if (ln.find("self.done = True") != std::string::npos)
            has_self_done = true;
    }
    CHECK(has_self_done);
}

TEST_CASE("replace_py_block: def __init__ with return annotation '-> None'")
{
    fs::path tmp = fs::temp_directory_path() /
                   "chunk_test_replace_py_block_arrow_none";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "dialog.py";
    {
        std::ofstream out(f);
        out << "from typing import Callable, Optional\n"
               "from PySide6 import QtCore, QtWidgets\n"
               "\n"
               "class DebugFramegraphTextureDialog(QtWidgets.QDialog):\n"
               "    def __init__(\n"
               "        self,\n"
               "        graphics: GraphicsBackend,\n"
               "        viewport: Viewport,\n"
               "        resource_name: str = \"debug\",\n"
               "        parent: Optional[QtWidgets.QWidget] = None,\n"
               "        get_available_resources: Optional[Callable[[], list[str]]] = None,\n"
               "        set_source_resource: Optional[Callable[[str], None]] = None,\n"
               "        get_paused: Optional[Callable[[], bool]] = None,\n"
               "        set_paused: Optional[Callable[[bool], None]] = None,\n"
               "    ) -> None:\n"
               "        super().__init__(parent)\n"
               "\n"
               "        # инициализируем список ресурсов и состояние паузы\n"
               "        self._graphics = graphics\n"
               "        self._viewport = viewport\n"
               "        self._resource_name = resource_name\n"
               "\n"
               "        self._update_resource_list()\n"
               "        self._sync_pause_state()\n";
    }

    fs::path patch = tmp / "patch_arrow_none.yaml";
    {
        std::ofstream out(patch);
        out << "language: python\n"
               "operations:\n"
               "  - op: replace_py_block\n"
               "    path: \"" << yaml_path(f) << "\"\n"
               "    marker: |\n"
               "      def __init__(\n"
               "          self,\n"
               "          graphics: GraphicsBackend,\n"
               "          viewport: Viewport,\n"
               "          resource_name: str = \"debug\",\n"
               "          parent: Optional[QtWidgets.QWidget] = None,\n"
               "          get_available_resources: Optional[Callable[[], list[str]]] = None,\n"
               "          set_source_resource: Optional[Callable[[str], None]] = None,\n"
               "          get_paused: Optional[Callable[[], bool]] = None,\n"
               "          set_paused: Optional[Callable[[bool], None]] = None,\n"
               "      ) -> None:\n"
               "    payload: |\n"
               "      def __init__(\n"
               "          self,\n"
               "          graphics: GraphicsBackend,\n"
               "          viewport: Viewport,\n"
               "          resource_name: str = \"debug\",\n"
               "          parent: Optional[QtWidgets.QWidget] = None,\n"
               "          get_available_resources: Optional[Callable[[], list[str]]] = None,\n"
               "          set_source_resource: Optional[Callable[[str], None]] = None,\n"
               "          get_paused: Optional[Callable[[], bool]] = None,\n"
               "          set_paused: Optional[Callable[[bool], None]] = None,\n"
               "      ) -> None:\n"
               "          # новый инициализатор с аннотацией -> None\n"
               "          super().__init__(parent)\n"
               "          self._graphics = graphics\n"
               "          self._viewport = viewport\n"
               "          self._resource_name = resource_name\n"
               "          self._update_resource_list()\n"
               "          self._sync_pause_state()\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);

    bool has_old_comment = false;
    bool has_new_comment = false;
    bool has_arrow_none = false;

    for (const auto &ln : lines)
    {
        if (ln.find("инициализируем список ресурсов") != std::string::npos)
            has_old_comment = true;
        if (ln.find("новый инициализатор с аннотацией") != std::string::npos)
            has_new_comment = true;
        if (ln.find(") -> None:") != std::string::npos)
            has_arrow_none = true;
    }

    CHECK(!has_old_comment);
    CHECK(has_new_comment);
    CHECK(has_arrow_none);
}
