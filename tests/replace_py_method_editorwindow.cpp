#include "guard/guard.h"
#include "apply.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static int run_apply_editorwindow(const fs::path &patch)
{
    std::string arg0 = "apply";
    std::string arg1 = "--quiet";
    std::string arg2 = patch.string();

    std::vector<std::string> args = {arg0, arg1, arg2};
    std::vector<char *> argv_real;
    argv_real.reserve(args.size());
    for (auto &s : args)
        argv_real.push_back(s.data());

    return apply_chunk_main((int)argv_real.size(), argv_real.data());
}

static std::vector<std::string> read_lines(const fs::path &p)
{
    std::ifstream in(p);
    std::vector<std::string> v;
    std::string s;
    while (std::getline(in, s))
        v.push_back(s);
    return v;
}

static std::string yaml_path(const fs::path &p)
{
    return p.generic_string();
}

TEST_CASE("apply_chunk_main: replace_py_method EditorWindow.push_undo_command is found")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_replace_py_method_editorwindow";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "editor_window.py";
    {
        std::ofstream out(f);
        out << "class EditorWindow(QMainWindow):\n";
        out << "    def __init__(self, world, scene):\n";
        out << "        super().__init__()\n";
        out << "        self.selected_entity_id = 0\n";
        out << "        self.hover_entity_id = 0\n";
        out << "        self.undo_stack = UndoStack()\n";
        out << "        self._action_undo = None\n";
        out << "        self._action_redo = None\n";
        out << "        self._undo_stack_viewer = None\n";
        out << "\n";
        out << "        self.world = world\n";
        out << "        self.scene = scene\n";
        out << "\n";
        out << "        # контроллеры создадим чуть позже\n";
        out << "        self.scene_tree_controller: SceneTreeController | None = None\n";
        out << "        self.viewport_controller: ViewportController | None = None\n";
        out << "        self.gizmo_controller: GizmoController | None = None\n";
        out << "\n";
        out << "        ui_path = os.path.join(os.path.dirname(__file__), \"editor.ui\")\n";
        out << "        uic.loadUi(ui_path, self)\n";
        out << "\n";
        out << "        self._setup_menu_bar()\n";
        out << "\n";
        out << "        # --- ресурс-менеджер редактора ---\n";
        out << "        self.resource_manager = ResourceManager()\n";
        out << "        self._init_resources_from_scene()\n";
        out << "\n";
        out << "        # --- UI из .ui ---\n";
        out << "        self.sceneTree: QTreeView = self.findChild(QTreeView, \"sceneTree\")\n";
        out << "\n";
        out << "        self.sceneTree.setContextMenuPolicy(Qt.CustomContextMenu)\n";
        out << "\n";
        out << "        self.viewportContainer: QWidget = self.findChild(QWidget, \"viewportContainer\")\n";
        out << "        self.inspectorContainer: QWidget = self.findChild(QWidget, \"inspectorContainer\")\n";
        out << "\n";
        out << "        from PyQt5.QtWidgets import QSplitter\n";
        out << "        self.topSplitter: QSplitter = self.findChild(QSplitter, \"topSplitter\")\n";
        out << "        self.verticalSplitter: QSplitter = self.findChild(QSplitter, \"verticalSplitter\")\n";
        out << "\n";
        out << "        self._fix_splitters()\n";
        out << "\n";
        out << "        # --- инспектор ---\n";
        out << "        self.inspector = EntityInspector(self.resource_manager, self.inspectorContainer)\n";
        out << "        self._init_inspector_widget()\n";
        out << "\n";
        out << "        component_library = [\n";
        out << "            (\"PerspectiveCameraComponent\", PerspectiveCameraComponent),\n";
        out << "            (\"OrbitCameraController\",      OrbitCameraController),\n";
        out << "            (\"MeshRenderer\",               MeshRenderer),\n";
        out << "        ]\n";
        out << "        self.inspector.set_component_library(component_library)\n";
        out << "\n";
        out << "        for label, cls in component_library:\n";
        out << "            self.resource_manager.register_component(label, cls)\n";
        out << "\n";
        out << "        self.inspector.transform_changed.connect(self._on_inspector_transform_changed)\n";
        out << "        self.inspector.component_changed.connect(self._on_inspector_component_changed)\n";
        out << "        self.inspector.set_undo_command_handler(self.push_undo_command)\n";
        out << "\n";
        out << "        # --- создаём редакторские сущности (root, камера) ---\n";
        out << "        self.editor_entities = None\n";
        out << "        self.camera = None\n";
        out << "        self._ensure_editor_entities_root()\n";
        out << "        self._ensure_editor_camera()\n";
        out << "\n";
        out << "        # --- гизмо-контроллер ---\n";
        out << "        self.gizmo_controller = GizmoController(\n";
        out << "            scene=self.scene,\n";
        out << "            editor_entities=self.editor_entities,\n";
        out << "            undo_handler=self.push_undo_command,\n";
        out << "        )\n";
        out << "\n";
        out << "        # --- дерево сцены ---\n";
        out << "        self.scene_tree_controller = SceneTreeController(\n";
        out << "            tree_view=self.sceneTree,\n";
        out << "            scene=self.scene,\n";
        out << "            undo_handler=self.push_undo_command,\n";
        out << "            on_object_selected=self._on_tree_object_selected,\n";
        out << "            request_viewport_update=self._request_viewport_update,\n";
        out << "        )\n";
        out << "\n";
        out << "        # --- viewport ---\n";
        out << "        self.viewport_window = None\n";
        out << "        self.viewport = None\n";
        out << "        self.viewport_controller = ViewportController(\n";
        out << "            container=self.viewportContainer,\n";
        out << "            world=self.world,\n";
        out << "            scene=self.scene,\n";
        out << "            camera=self.camera,\n";
        out << "            gizmo_controller=self.gizmo_controller,\n";
        out << "            on_entity_picked=self._on_entity_picked_from_viewport,\n";
        out << "            on_hover_entity=self._on_hover_entity_from_viewport,\n";
        out << "        )\n";
        out << "\n";
        out << "        self.viewport_window = self.viewport_controller.window\n";
        out << "        self.viewport = self.viewport_controller._backend.viewport  # внутренняя ссылка для совместимости\n";
        out << "\n";
        out << "        gl_widget = self.viewport_controller.gl_widget\n";
        out << "        gl_widget.installEventFilter(self)\n";
        out << "\n";
        out << "        self._selected_entity: Entity | None = None\n";
        out << "\n";
        out << "    # ----------- undo / redo -----------\n";
        out << "\n";
        out << "    def _setup_menu_bar(self) -> None:\n";
        out << "        \"\"\"\n";
        out << "        Создаёт верхнее меню редактора и вешает действия Undo/Redo с шорткатами.\n";
        out << "        Также добавляет отладочное меню Debug с просмотром undo/redo стека.\n";
        out << "        \"\"\"\n";
        out << "        menu_bar = self.menuBar()\n";
        out << "\n";
        out << "        file_menu = menu_bar.addMenu(\"File\")\n";
        out << "        edit_menu = menu_bar.addMenu(\"Edit\")\n";
        out << "        debug_menu = menu_bar.addMenu(\"Debug\")\n";
        out << "\n";
        out << "        exit_action = file_menu.addAction(\"Exit\")\n";
        out << "        exit_action.setShortcut(\"Ctrl+Q\")\n";
        out << "        exit_action.triggered.connect(self.close)\n";
        out << "\n";
        out << "        self._action_undo = edit_menu.addAction(\"Undo\")\n";
        out << "        self._action_undo.setShortcut(\"Ctrl+Z\")\n";
        out << "        self._action_undo.triggered.connect(self.undo)\n";
        out << "\n";
        out << "        self._action_redo = edit_menu.addAction(\"Redo\")\n";
        out << "        self._action_redo.setShortcut(\"Ctrl+Shift+Z\")\n";
        out << "        self._action_redo.triggered.connect(self.redo)\n";
        out << "\n";
        out << "        debug_action = debug_menu.addAction(\"Undo/Redo Stack...\")\n";
        out << "        debug_action.triggered.connect(self._show_undo_stack_viewer)\n";
        out << "\n";
        out << "        self._update_undo_redo_actions()\n";
        out << "\n";
        out << "    def _update_undo_redo_actions(self) -> None:\n";
        out << "        \"\"\"\n";
        out << "        Обновляет enabled-состояние пунктов меню Undo/Redo.\n";
        out << "        \"\"\"\n";
        out << "        if self._action_undo is not None:\n";
        out << "            self._action_undo.setEnabled(self.undo_stack.can_undo)\n";
        out << "        if self._action_redo is not None:\n";
        out << "            self._action_redo.setEnabled(self.undo_stack.can_redo)\n";
        out << "\n";
        out << "    def push_undo_command(self, cmd: UndoCommand, merge: bool = False) -> None:\n";
        out << "        \"\"\"\n";
        out << "        Добавить команду в undo-стек редактора.\n";
        out << "        merge=True — попытаться слить с предыдущей (для крутилок трансформа).\n";
        out << "        \"\"\"\n";
        out << "        self.undo_stack.push(cmd, merge=merge)\n";
        out << "        self._request_viewport_update()\n";
        out << "        self._update_undo_redo_actions()\n";
        out << "\n";
        out << "    def undo(self) -> None:\n";
        out << "        cmd = self.undo_stack.undo()\n";
        out << "        select_obj = None\n";
        out << "\n";
        out << "        if isinstance(cmd, AddEntityCommand):\n";
        out << "            select_obj = cmd.parent_entity\n";
        out << "        elif isinstance(cmd, DeleteEntityCommand):\n";
        out << "            select_obj = cmd.entity\n";
        out << "        elif isinstance(cmd, RenameEntityCommand):\n";
        out << "            select_obj = cmd.entity\n";
        out << "\n";
        out << "        if self.scene_tree_controller is not None:\n";
        out << "            self.scene_tree_controller.rebuild(select_obj=select_obj)\n";
        out << "\n";
        out << "        self._request_viewport_update()\n";
        out << "        self._resync_inspector_from_selection()\n";
        out << "        self._update_undo_redo_actions()\n";
    }

    fs::path patch = tmp / "patch.yaml";
    {
        std::ofstream out(patch);
        out << "operations:\n";
        out << "  - op: replace_py_method\n";
        out << "    comment: \"Эмитим сигнал undo_stack_changed при добавлении новой команды в стек\"\n";
        out << "    path: \"" << yaml_path(f) << "\"\n";
        out << "    class: EditorWindow\n";
        out << "    method: push_undo_command\n";
        out << "    payload: |\n";
        out << "      def push_undo_command(self, cmd: UndoCommand, merge: bool = False) -> None:\n";
        out << "          \"\"\"\n";
        out << "          Добавить команду в undo-стек редактора.\n";
        out << "          merge=True — попытаться слить с предыдущей (для крутилок трансформа).\n";
        out << "          \"\"\"\n";
        out << "          self.undo_stack.push(cmd, merge=merge)\n";
        out << "          self._request_viewport_update()\n";
        out << "          self._update_undo_redo_actions()\n";
        out << "          self.undo_stack_changed.emit()\n";
    }

    int r = run_apply_editorwindow(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    bool found = false;
    for (const auto &line : lines)
    {
        if (line.find("undo_stack_changed.emit()") != std::string::npos)
        {
            found = true;
            break;
        }
    }
    CHECK(found);
}
