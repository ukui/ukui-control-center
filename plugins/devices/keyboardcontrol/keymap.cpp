#include "keymap.h"

KeyMap::KeyMap()
{
    metaColor = QMetaEnum::fromType<KeyMap::CCKey>();
}

KeyMap::~KeyMap()
{
}

QString KeyMap::keycodeTokeystring(int code){
    return metaColor.valueToKey(code);    //未匹配到则返回空
}
