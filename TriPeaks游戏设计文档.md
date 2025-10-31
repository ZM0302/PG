# TriPeaks 纸牌游戏设计文档

## 一、架构概述

本项目采用 **MVC（Model-View-Controller）架构模式**，将视图、逻辑和数据完全分离，确保代码的可维护性和可扩展性。整个项目遵循分层设计原则，各层职责明确，依赖关系清晰。

### 架构层次图

```
┌─────────────────────────────────────────┐
│            View Layer (视图层)           │
│         GameView, CardVisual            │
│    只负责UI展示和用户输入接收             │
└─────────────────────────────────────────┘
                    ↕ (回调函数)
┌─────────────────────────────────────────┐
│        Controller Layer (控制器层)       │
│  GameController, PlayFieldController,   │
│        StackController                  │
│    处理业务逻辑，协调Model和View          │
└─────────────────────────────────────────┘
         ↕                    ↕
┌──────────────────┐  ┌──────────────────┐
│  Manager Layer   │  │  Service Layer   │
│  (管理器层)      │  │  (服务层)        │
│  UndoManager    │  │  CardMatchService │
│  可持有Model数据 │  │  无状态服务      │
└──────────────────┘  └──────────────────┘
                    ↕
┌─────────────────────────────────────────┐
│         Model Layer (数据模型层)          │
│  GameModel, Card, UndoMove              │
│    存储游戏数据和状态                     │
└─────────────────────────────────────────┘
```

## 二、目录结构

```
Classes/
├── configs/          # 静态配置相关类
│   ├── models/       # 配置数据模型
│   │   └── LevelConfig.h
│   └── loaders/      # 配置加载逻辑
│       └── LevelConfigLoader.h
│
├── models/           # 运行时动态数据模型
│   ├── GameModel.h/cpp      # 游戏核心数据模型
│   └── UndoMove.h           # 回退数据结构
│
├── views/            # 视图层，UI展示组件
│   └── GameView.h/cpp       # 游戏视图主类
│
├── controllers/      # 控制器层，协调模型和视图
│   ├── GameController.h/cpp        # 游戏主控制器
│   ├── PlayFieldController.h/cpp   # 主牌区控制器
│   └── StackController.h/cpp        # 备用牌堆控制器
│
├── managers/         # 管理器层，提供全局性服务
│   └── UndoManager.h/cpp           # 回退管理器
│
└── services/         # 服务层，无状态业务逻辑
    ├── CardMatchService.h/cpp              # 卡牌匹配服务
    └── GameModelFromLevelGenerator.h/cpp   # 关卡数据生成服务
```

## 三、各模块职责详解

### 3.1 configs/ - 静态配置层

**职责和边界：**
- 负责所有静态配置相关类的定义和加载
- 不包含运行时动态数据
- 配置数据在游戏初始化时加载，后续只读使用

**关键类：**

#### LevelConfig.h
- **职责**：定义关卡配置的数据结构
- **包含**：
  - `CardSuit`：卡牌花色枚举（梅花、方块、红桃、黑桃）
  - `CardFaceType`：卡牌面值枚举（A, 2-K）
  - `LevelCardConfig`：单张卡牌的配置信息
  - `LevelConfig`：整个关卡的配置信息

#### LevelConfigLoader.h/cpp
- **职责**：从JSON文件加载关卡配置
- **功能**：解析JSON配置，转换为 `LevelConfig` 对象

**示例用法：**
```cpp
LevelConfig levelConfig;
LevelConfigLoader::loadFromFile("levels/level_tripeaks_standard.json", levelConfig);
```

### 3.2 models/ - 数据模型层

**职责和边界：**
- 存储游戏运行时数据和状态
- 不包含复杂的业务逻辑
- 提供数据访问和修改接口
- 支持序列化和反序列化（为存档功能预留）

**关键类：**

#### GameModel.h/cpp
- **职责**：游戏核心数据模型，管理所有卡牌状态
- **核心数据结构**：
  - `_cards`：所有卡牌的集合
  - `_playfieldCardIds`：主牌区卡牌ID列表
  - `_stockCardIds`：备用牌堆卡牌ID列表
  - `_trayCardId`：手牌区顶部牌ID（单张牌）
- **核心方法**：
  - `getCardById()`：根据ID获取卡牌
  - `isCardExposed()`：判断卡牌是否可操作
  - `replaceTrayCard()`：替换手牌区顶部牌
  - `removeCardFromPlayfield()`：从主牌区移除卡牌

#### UndoMove.h
- **职责**：定义回退操作的数据结构
- **核心字段**：
  - `type`：操作类型（PlayfieldMatch / ReplaceTrayFromStock）
  - `movedCardId`：移动的卡牌ID
  - `previousTrayCardId`：之前的手牌区顶部牌ID
  - `previousPlayfieldIndex`：桌面牌在原位置索引
  - `previousStockIndex`：stock牌在原位置索引
  - `flipStates`：自动翻开的卡牌状态列表

**数据流向：**
```
配置文件 → GameModelFromLevelGenerator → GameModel
用户操作 → Controller → GameModel (更新状态)
回退操作 → UndoManager → UndoMove → GameModel (恢复状态)
```

### 3.3 views/ - 视图层

**职责和边界：**
- 只负责界面展示和接收用户输入
- 不包含业务逻辑
- 可持有 `const` 类型的 controller 指针和 `const` 类型的 model 指针
- 通过回调函数与 Controller 通信

**关键类：**

#### GameView.h/cpp
- **职责**：游戏主视图，管理所有卡牌的视觉表现
- **核心结构**：
  - `CardVisual`：单张卡牌的视觉表现结构
  - `_cardLayer`：卡牌渲染层
  - `_uiLayer`：UI元素层
- **核心方法**：
  - `buildInitialLayout()`：构建初始布局
  - `replaceTrayCardWithPlayfieldCard()`：桌面牌替换手牌区动画
  - `replaceTrayCardWithStockCard()`：stock牌替换手牌区动画
  - `undoReplaceTrayCard()`：回退动画
  - `flipCard()`：卡牌翻转动画

**视图更新流程：**
```
Controller 调用 View 的动画方法
    ↓
View 更新 CardVisual 状态
    ↓
View 执行 Cocos2d 动画
    ↓
动画完成后刷新卡牌状态
```

### 3.4 controllers/ - 控制器层

**职责和边界：**
- 处理用户操作和业务逻辑
- 连接视图和模型
- 协调多个 Services 和 Managers
- 不直接操作渲染，通过 View 接口执行动画

**关键类：**

#### GameController.h/cpp
- **职责**：游戏主控制器，管理整个游戏流程
- **核心功能**：
  - 初始化游戏（加载关卡、创建视图）
  - 处理用户输入（卡牌点击、stock点击、回退点击）
  - 协调子控制器
  - 管理回退栈
- **成员变量**：
  - `_model`：游戏数据模型
  - `_view`：游戏视图
  - `_undoManager`：回退管理器
  - `_playfieldController`：主牌区控制器
  - `_stackController`：备用牌堆控制器

#### PlayFieldController.h/cpp
- **职责**：处理主牌区的卡牌操作
- **核心方法**：
  - `handleCardTap()`：处理桌面牌点击
  - `undoMatch()`：回退桌面牌匹配操作

#### StackController.h/cpp
- **职责**：处理备用牌堆的操作
- **核心方法**：
  - `handleStockTap()`：处理stock点击
  - `undoDraw()`：回退stock翻牌操作
  - `drawInitialCard()`：抽取初始手牌区牌

**控制器处理流程：**
```
用户点击 → View 回调 → Controller.handleXXX()
    ↓
Controller 验证规则（通过 Service）
    ↓
Controller 更新 Model
    ↓
Controller 记录回退信息（通过 UndoManager）
    ↓
Controller 调用 View 执行动画
    ↓
Controller 检查胜利条件
```

### 3.5 managers/ - 管理器层

**职责和边界：**
- 主要作为 Controller 的成员变量
- 可持有 Model 数据并对数据进行加工
- **禁止实现为单例模式**
- **禁止反向依赖 Controller**
- 与其他模块交互通过回调接口实现

**关键类：**

#### UndoManager.h/cpp
- **职责**：管理回退操作栈
- **核心方法**：
  - `push()`：添加回退记录
  - `pop()`：弹出回退记录
  - `canUndo()`：判断是否可以回退
  - `clear()`：清空回退栈
- **数据结构**：
  - `_moves`：回退操作栈（vector）

**使用示例：**
```cpp
UndoMove move;
if (controller.handleCardTap(cardId, move)) {
    _undoManager.push(move);  // 记录操作
}

// 回退时
UndoMove move;
if (_undoManager.pop(move)) {
    controller.undoMatch(move);  // 执行回退
}
```

### 3.6 services/ - 服务层

**职责和边界：**
- 提供无状态的服务，处理业务逻辑
- **不管理数据生命周期**（不持有数据）
- 通过参数操作数据或返回数据
- 可以实现为单例或提供静态方法
- 不依赖 Controllers，提供基础服务

**关键类：**

#### CardMatchService.h/cpp
- **职责**：卡牌匹配规则服务
- **核心方法**：
  - `canMatch()`：判断两张牌是否可以匹配（点数差1）
  - `hasMatchableCardInPlayfield()`：检查主牌区是否有可匹配的牌
  - `randomizeCard()`：随机生成卡牌

#### GameModelFromLevelGenerator.h/cpp
- **职责**：将静态配置转换为运行时数据模型
- **核心方法**：
  - `generateFromLevel()`：从关卡配置生成 GameModel
- **处理逻辑**：
  - 解析 LevelConfig
  - 创建 Card 对象
  - 分配卡牌位置和状态
  - 建立卡牌覆盖关系

**服务层特点：**
- 所有方法都是静态方法或通过参数传递数据
- 不持有任何状态
- 可以被多个 Controller 复用

## 四、组件间通信流程

### 4.1 用户UI交互流程

#### 4.1.1 点击桌面牌匹配流程

```
用户点击桌面牌
    ↓
GameView::attachCardListener 捕获触摸事件
    ↓
调用 _onCardTapped(cardId) 回调
    ↓
GameController::onCardTapped(cardId)
    ↓
PlayFieldController::handleCardTap(cardId, move)
    ├─ 验证卡牌是否可操作（通过 Model）
    ├─ 验证匹配规则（通过 CardMatchService）
    ├─ 更新 Model（替换tray card，移除桌面牌）
    ├─ 记录回退信息（UndoMove）
    └─ 调用 View 执行动画
        ↓
GameView::replaceTrayCardWithPlayfieldCard()
    ├─ 更新 CardVisual 状态
    └─ 执行 MoveTo 动画
    ↓
UndoManager::push(move) 记录操作
    ↓
GameController::handleVictoryCheck() 检查胜利
```

#### 4.1.2 点击stock翻牌流程

```
用户点击stock区域
    ↓
GameView::_stockTouchNode 捕获触摸事件
    ↓
调用 _onStockTapped() 回调
    ↓
GameController::onStockTapped()
    ↓
StackController::handleStockTap(move)
    ├─ 从 Model 抽取stock牌
    ├─ 更新 Model（替换tray card）
    ├─ 记录回退信息（UndoMove）
    └─ 调用 View 执行动画
        ↓
GameView::replaceTrayCardWithStockCard()
    ├─ 更新 CardVisual 状态
    └─ 执行 MoveTo 动画
    ↓
UndoManager::push(move) 记录操作
```

#### 4.1.3 回退操作流程

```
用户点击回退按钮
    ↓
GameView::Undo菜单项捕获点击
    ↓
调用 _onUndoTapped() 回调
    ↓
GameController::onUndoTapped()
    ↓
UndoManager::pop(move) 获取回退记录
    ↓
根据 move.type 分发到对应 Controller
    ├─ PlayfieldMatch → PlayFieldController::undoMatch()
    └─ ReplaceTrayFromStock → StackController::undoDraw()
        ↓
Controller 恢复 Model 状态
    ↓
Controller 调用 View 执行回退动画
    ↓
GameView::undoReplaceTrayCard() 或 undoReplaceTrayCardFromStock()
    └─ 执行反向 MoveTo 动画
```

### 4.2 游戏初始化流程

```
用户启动游戏
    ↓
HelloWorldScene::init()
    ↓
创建 GameView 和 GameController
    ↓
GameController::init(view, levelPath)
    ├─ LevelConfigLoader::loadFromFile() 加载配置
    ├─ GameModelFromLevelGenerator::generateFromLevel() 生成Model
    ├─ 初始化各子控制器
    ├─ View::buildInitialLayout() 构建布局
    └─ StackController::drawInitialCard() 抽取初始tray牌
        ↓
显示游戏界面
```

## 五、扩展指南

### 5.1 如何新增一种卡牌类型

假设需要添加一种特殊卡牌（如"万能牌"），需要修改以下模块：

#### 步骤1：扩展配置层（configs/models/LevelConfig.h）

```cpp
// 在 LevelConfig.h 中添加新的卡牌类型枚举
enum class SpecialCardType
{
    None = 0,
    WildCard,      // 万能牌
    BombCard       // 炸弹牌
};

// 扩展 LevelCardConfig 结构
struct LevelCardConfig
{
    // ... 现有字段 ...
    SpecialCardType specialType = SpecialCardType::None;  // 新增字段
};
```

#### 步骤2：扩展数据模型层（models/GameModel.h）

```cpp
// 在 Card 结构中添加新字段
struct Card
{
    // ... 现有字段 ...
    SpecialCardType specialType = SpecialCardType::None;  // 新增字段
};

// 在 GameModel 中添加新方法
class GameModel
{
    // ... 现有方法 ...
    void setCardSpecialType(int cardId, SpecialCardType type);
    SpecialCardType getCardSpecialType(int cardId) const;
};
```

#### 步骤3：扩展服务层（services/CardMatchService.h）

```cpp
class CardMatchService
{
    // ... 现有方法 ...
    // 新增万能牌匹配规则
    static bool canMatchWithWildCard(CardFaceType faceA, CardFaceType faceB, bool hasWildCard);
    
    // 检查是否有万能牌
    static bool hasWildCardInPlayfield(const GameModel& model);
};
```

#### 步骤4：扩展控制器层（controllers/PlayFieldController.cpp）

```cpp
bool PlayFieldController::handleCardTap(int cardId, UndoMove& outMove)
{
    // ... 现有验证逻辑 ...
    
    // 新增：检查是否是万能牌
    const Card* card = _model->getCardById(cardId);
    bool isWildCard = (card && card->specialType == SpecialCardType::WildCard);
    
    // 修改匹配规则判断
    bool canMatch = false;
    if (isWildCard) {
        canMatch = true;  // 万能牌可以匹配任何牌
    } else {
        canMatch = CardMatchService::canMatch(card->face, trayCard->face);
    }
    
    // ... 后续处理逻辑 ...
}
```

#### 步骤5：扩展视图层（views/GameView.cpp）

```cpp
// 在 createCardVisual 中添加特殊卡牌的视觉表现
GameView::CardVisual GameView::createCardVisual(const Card& card)
{
    // ... 现有创建逻辑 ...
    
    // 新增：如果是万能牌，添加特殊标记
    if (card.specialType == SpecialCardType::WildCard) {
        auto wildMarker = cocos2d::Sprite::create("res/wild_marker.png");
        wildMarker->setPosition(0.0F, 0.0F);
        visual.front->addChild(wildMarker, 10);
    }
    
    return visual;
}
```

#### 步骤6：扩展回退数据（models/UndoMove.h）

```cpp
struct UndoMove
{
    // ... 现有字段 ...
    bool usedWildCard = false;  // 记录是否使用了万能牌
};
```

**扩展要点总结：**
1. 配置层：添加新类型枚举和配置字段
2. 模型层：添加数据字段和访问方法
3. 服务层：添加新的业务规则判断
4. 控制器层：修改业务逻辑处理
5. 视图层：添加视觉表现
6. 回退数据：记录新类型的状态

### 5.2 如何新增一种回退功能类型

假设需要添加"撤销卡牌翻转"的回退功能：

#### 步骤1：扩展回退数据类型（models/UndoMove.h）

```cpp
struct UndoMove
{
    enum class Type
    {
        PlayfieldMatch,           // 现有类型
        ReplaceTrayFromStock,     // 现有类型
        FlipCard                  // 新增类型：翻转卡牌
    };

    Type type = Type::PlayfieldMatch;
    // ... 现有字段 ...
    
    // 新增字段（仅FlipCard类型使用）
    int flippedCardId = -1;       // 被翻转的卡牌ID
    bool previousFaceUp = false; // 之前的正面状态
};
```

#### 步骤2：创建新的控制器方法（controllers/PlayFieldController.h）

```cpp
class PlayFieldController
{
    // ... 现有方法 ...
    
    // 新增：处理卡牌翻转
    bool handleCardFlip(int cardId, UndoMove& outMove);
    
    // 新增：回退卡牌翻转
    void undoFlip(const UndoMove& move);
};
```

#### 步骤3：实现控制器逻辑（controllers/PlayFieldController.cpp）

```cpp
bool PlayFieldController::handleCardFlip(int cardId, UndoMove& outMove)
{
    if (!_model || !_view) {
        return false;
    }

    Card* card = _model->getCardById(cardId);
    if (!card || card->removed) {
        return false;
    }

    // 记录回退信息
    outMove = UndoMove{};
    outMove.type = UndoMove::Type::FlipCard;
    outMove.flippedCardId = cardId;
    outMove.previousFaceUp = card->faceUp;

    // 翻转卡牌
    _model->setCardFaceUp(cardId, !card->faceUp);
    _view->flipCard(cardId, !card->faceUp);

    return true;
}

void PlayFieldController::undoFlip(const UndoMove& move)
{
    if (!_model || !_view) {
        return;
    }

    if (move.type != UndoMove::Type::FlipCard) {
        return;
    }

    // 恢复卡牌状态
    _model->setCardFaceUp(move.flippedCardId, move.previousFaceUp);
    _view->flipCard(move.flippedCardId, move.previousFaceUp);
}
```

#### 步骤4：在主控制器中注册新类型（controllers/GameController.cpp）

```cpp
void GameController::onUndoTapped()
{
    UndoMove move;
    if (!_undoManager.pop(move)) {
        if (_view) {
            _view->showStatusMessage("Nothing to undo");
        }
        return;
    }

    switch (move.type)
    {
    case UndoMove::Type::PlayfieldMatch:
        _playfieldController.undoMatch(move);
        break;
    case UndoMove::Type::ReplaceTrayFromStock:
        _stackController.undoDraw(move);
        break;
    case UndoMove::Type::FlipCard:  // 新增分支
        _playfieldController.undoFlip(move);
        break;
    default:
        break;
    }

    handleVictoryCheck();
}
```

#### 步骤5：添加用户交互入口（views/GameView.h）

```cpp
class GameView
{
    // ... 现有方法 ...
    
    // 新增：设置卡牌翻转回调
    void setCardFlipCallback(const std::function<void(int)>& callback);
    
private:
    std::function<void(int)> _onCardFlipped;  // 新增回调
};
```

**扩展要点总结：**
1. 在 `UndoMove` 中添加新的操作类型枚举
2. 添加新类型所需的回退数据字段
3. 在对应的 Controller 中实现处理逻辑和回退逻辑
4. 在 `GameController` 的回退分发中添加新分支
5. 在 View 中添加用户交互入口（如需要）

### 5.3 扩展性设计原则

1. **单一职责原则**：每个类只负责一个明确的功能
2. **开闭原则**：对扩展开放，对修改关闭
3. **依赖倒置**：高层模块不依赖低层模块，都依赖抽象
4. **接口隔离**：使用回调函数而非直接依赖
5. **数据驱动**：通过配置文件驱动游戏逻辑

## 六、编码规范

### 6.1 命名规范

- **类名和文件名**：大写字母开头（PascalCase）
  - 示例：`GameController`, `CardMatchService`
- **函数名和变量名**：小写字母开头（camelCase）
  - 示例：`handleCardTap()`, `cardId`
- **类的私有成员和方法**：以下划线 `_` 开头
  - 示例：`_model`, `_cardVisuals`, `_onCardTapped`
- **常量变量名**：以小写字母 `k` 开头
  - 示例：`kDesignWidth`, `kDesignHeight`

### 6.2 代码质量要求

1. **注释要求**：
   - 每个类必须在声明处添加类的注释，清晰描述类的功能、职责和使用场景
   - 类的成员变量和公共方法必须添加规范的注释，说明其用途、参数和返回值

2. **代码长度限制**：
   - 当函数代码超过50行，请重构
   - 当类代码超过500行，请重构

3. **模块职责明确**：
   - 遵循单一职责原则
   - 要求代码的可维护性和可扩展性

### 6.3 各模块具体规范

#### models层
- 数据层，不包含复杂的业务逻辑
- 支持序列化和反序列化（为存档功能预留）
- 提供清晰的数据访问接口

#### views层
- UI视图层，负责界面展示
- 可持有 `const` 类型的 controller 指针和 `const` 类型的 model 指针
- 与其他模块的交互通过回调接口实现
- 不包含业务逻辑判断

#### controllers层
- 主要协调 model 和 view 之间的交互
- 处理用户操作的业务逻辑
- 通过 Service 验证规则
- 通过 Manager 管理状态

#### managers层
- 主要作为 controller 的成员变量
- 可持有 model 数据并对 model 数据进行加工
- **禁止实现为单例模式**
- **禁止反向依赖 controller**
- 与其他模块的交互通过回调接口实现

#### services层
- 处理业务逻辑，不管理数据生命周期
- **自身禁止持有数据**，但可加工数据
- 可以实现为单例或提供静态方法
- 不依赖 Controllers，提供基础服务

#### utils层
- 提供通用独立的辅助功能
- 不涉及业务逻辑，完全独立

## 七、数据流图

### 7.1 卡牌匹配数据流

```
[LevelConfig JSON]
    ↓
[LevelConfigLoader] → [LevelConfig对象]
    ↓
[GameModelFromLevelGenerator] → [GameModel]
    ↓
[PlayFieldController] ← [CardMatchService] (验证规则)
    ↓
[GameModel] (更新状态)
    ↓
[UndoManager] (记录操作)
    ↓
[GameView] (执行动画)
```

### 7.2 回退操作数据流

```
[UndoManager] (弹出回退记录)
    ↓
[UndoMove对象]
    ↓
[GameController] (分发到对应Controller)
    ↓
[PlayFieldController/StackController] (恢复Model状态)
    ↓
[GameView] (执行回退动画)
```

## 八、关键设计决策

### 8.1 为什么使用回调函数而非直接依赖？

**原因**：
- 降低耦合度：View 不需要知道具体的 Controller 类型
- 便于测试：可以轻松替换回调实现
- 符合依赖倒置原则：View 依赖抽象（回调接口）而非具体实现

### 8.2 为什么 Manager 禁止单例？

**原因**：
- 便于单元测试：可以创建多个实例进行测试
- 避免全局状态：每个 Controller 拥有自己的 Manager 实例
- 支持多实例场景：未来可能支持多局游戏

### 8.3 为什么 Service 可以单例或静态方法？

**原因**：
- Service 是无状态的，不持有数据
- 静态方法调用更方便，无需实例化
- 多个 Controller 可以共享同一个 Service

### 8.4 为什么回退数据单独定义？

**原因**：
- 回退数据是跨模块的，需要序列化保存
- 独立定义便于扩展新的回退类型
- 回退逻辑与业务逻辑分离，便于维护

## 九、测试建议

### 9.1 单元测试

- **Model层**：测试数据访问和状态更新
- **Service层**：测试业务规则判断
- **Manager层**：测试回退栈操作

### 9.2 集成测试

- **Controller + Model**：测试业务逻辑和数据更新
- **Controller + View**：测试交互流程
- **完整流程**：测试从用户操作到界面更新的完整流程

### 9.3 测试工具建议

- 使用 Google Test 或类似框架
- Mock View 和 Model 进行隔离测试
- 使用配置文件驱动测试用例

## 十、未来扩展方向

1. **存档功能**：序列化 GameModel 和 UndoManager 状态
2. **多关卡支持**：扩展 LevelConfigLoader 支持多关卡
3. **动画系统**：抽象动画接口，支持多种动画效果
4. **音效系统**：在 View 层添加音效播放接口
5. **AI玩家**：在 Service 层添加AI决策算法

---

## 附录：关键代码示例

### A.1 添加新卡牌类型的完整示例

参见"5.1 如何新增一种卡牌类型"章节。

### A.2 添加新回退类型的完整示例

参见"5.2 如何新增一种回退功能类型"章节。

### A.3 典型交互流程代码

```cpp
// GameController::onCardTapped 示例
void GameController::onCardTapped(int cardId)
{
    UndoMove move;
    if (!_playfieldController.handleCardTap(cardId, move)) {
        return;  // 操作失败，不记录回退
    }

    _undoManager.push(move);  // 记录操作
    handleVictoryCheck();      // 检查胜利
}
```

---

**文档版本**：v1.0  


