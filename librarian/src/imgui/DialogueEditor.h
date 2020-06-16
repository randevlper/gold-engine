#pragma once

namespace me {
	namespace ui {
		class Text;
	}
}

namespace lb {
	class DialogueWriter;
}

namespace lb {
	namespace imgui {
		void showDialogueEditor(me::ui::Text* text);
		void init(DialogueWriter* writer);
	}
}