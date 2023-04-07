#include "task.h"
#include "logged_command_wrapper.h"
#include <iostream>

CommandLoggerVisitor::CommandLoggerVisitor(std::ostream& logStream):logStream_(logStream) {}

void CommandLoggerVisitor::VisitMoveCursorLeftCommand(MoveCursorLeftCommand& command)
{
    logStream_ << "h";
}
    void CommandLoggerVisitor::VisitMoveCursorRightCommand(MoveCursorRightCommand& command)
    {
        logStream_ << "l";
    }
    void CommandLoggerVisitor::VisitMoveCursorUpCommand(MoveCursorUpCommand& command)
    {
logStream_ << "k";
    }
    void CommandLoggerVisitor::VisitMoveCursorDownCommand(MoveCursorDownCommand& command)
    {
logStream_ << "j";
    }
    void CommandLoggerVisitor::VisitSelectCommand(SelectTextCommand& command)
    {
logStream_ << "v";
    }
    void CommandLoggerVisitor::VisitInsertTextCommand(InsertTextCommand& command)
    {
        logStream_ << "i";
    }
    void CommandLoggerVisitor::VisitDeleteTextCommand(DeleteTextCommand& command)
    {
logStream_ << "d";
    }
    void CommandLoggerVisitor::VisitCopyTextCommand(CopyTextCommand& command)
    {
logStream_ << "y";
    }
    void CommandLoggerVisitor::VisitPasteTextCommand(PasteTextCommand& command)
    {
logStream_ << "p";
    }
    void CommandLoggerVisitor::VisitUppercaseTextCommand(UppercaseTextCommand& command)
    {
logStream_ << "U";
    }
    void CommandLoggerVisitor::VisitLowercaseTextCommand(LowercaseTextCommand& command)
    {
logStream_ << "u";
    }
    void CommandLoggerVisitor::VisitMoveToEndCommand(MoveToEndCommand& command)
    {
logStream_ << "$";
    }
    void CommandLoggerVisitor::VisitMoveToStartCommand(MoveToStartCommand& command)
    {
logStream_ << "0";
    }
    void CommandLoggerVisitor::VisitDeleteWordCommand(DeleteWordCommand& command)
    {
logStream_ << "dE";
    }




class MoveCursorLeftCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        cursorPosition=std::max((size_t)0,cursorPosition-1);
        editor.UnselectText();
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitMoveCursorLeftCommand(*this);
    }

};

/* Курсор вправо */
class MoveCursorRightCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        cursorPosition=std::min(buffer.size(),cursorPosition+1);
        editor.UnselectText();
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitMoveCursorRightCommand(*this);
    }
};

/* Курсор вверх */
class MoveCursorUpCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        size_t x,y;
        //std::cout << int(buffer.substr(0,cursorPosition).rfind("\n")) << std::endl;
        if(int(buffer.substr(0,cursorPosition).rfind('\n'))!=-1){
            x= buffer.substr(0,cursorPosition).rfind('\n')+1;
        }else{
            x=0;
        }
        if (x!=0){
            if (int(buffer.substr(0,x-1).rfind('\n'))!=-1){
                y=buffer.substr(0,x-1).rfind('\n')+1;
            }else{
                y=0;
            }
            //std::cout << y << " " << x << " " << cursorPosition << std::endl;
            cursorPosition=y+cursorPosition-x;
        }
        editor.UnselectText();
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitMoveCursorUpCommand(*this);
    }
};

/* Курсор вниз */
class MoveCursorDownCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        size_t x,y;
        //std::cout << int(buffer.substr(0,cursorPosition).rfind("\n")) << std::endl;
        if(int(buffer.substr(cursorPosition).find('\n'))!=-1){
            x= buffer.substr(cursorPosition).find('\n')+1;
            if (int(buffer.substr(0,cursorPosition).rfind('\n'))!=-1){
                y=buffer.substr(0,cursorPosition).rfind('\n')+1;
            }else{
                y=0;
            }
            cursorPosition=x+2*cursorPosition-y;
            if (cursorPosition>buffer.size()){
                cursorPosition=(cursorPosition-x+y)/2;
            }
        }
        editor.UnselectText();
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitMoveCursorDownCommand(*this);
    }
};

/* Выделить текст */
class SelectTextCommand : public ICommand {
public:
    SelectTextCommand(size_t& x) : sizeOfSelection(x) {}
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        editor.SelectText(cursorPosition,cursorPosition+sizeOfSelection);
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitSelectCommand(*this);
    }

private:
    size_t sizeOfSelection;
};

/* Ввести текст */
class InsertTextCommand : public ICommand {
public:
    InsertTextCommand() {}
    InsertTextCommand(std::string& text): text_1(text) {}
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        if (editor.HasSelection()){
            std::pair<size_t,size_t> x = editor.GetSelection();
            cursorPosition=x.first;
            buffer=buffer.substr(0,x.first)+buffer.substr(x.second);
            //cursorPosition-=(x.second-x.first);
            buffer=buffer.substr(0,cursorPosition)+text_1+buffer.substr(cursorPosition);
            cursorPosition+=text_1.size();
        }else{
            buffer=buffer.substr(0,cursorPosition)+text_1+buffer.substr(cursorPosition);
            cursorPosition+=text_1.size();
        }
    }
    void AcceptVisitor(CommandVisitor& visitor) override 
    {
        //InsertTextCommand I();
        //std::make_shared<InsertTextCommand>();
        visitor.VisitInsertTextCommand(*this);
    }
private:
    std::string text_1;
};

/* Удалить текст */
class DeleteTextCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        if (editor.HasSelection()){
            std::pair<size_t,size_t> x = editor.GetSelection();
            buffer=buffer.substr(0,x.first)+buffer.substr(x.second+1);
        }else{
            buffer=buffer.substr(0,cursorPosition)+buffer.substr(cursorPosition+1);
        }
        editor.UnselectText();
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitDeleteTextCommand(*this);
    }
};

/* Скопировать текст */
class CopyTextCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        if (editor.HasSelection()){
            std::pair<size_t,size_t> x = editor.GetSelection();
            clipboard=buffer.substr(x.first,x.second-x.first);
        }else{
            clipboard=buffer[cursorPosition];
        }
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitCopyTextCommand(*this);
    }
};

/* Вставить скопированный текст */
class PasteTextCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        if (editor.HasSelection()){
            std::pair<size_t,size_t> x = editor.GetSelection();
            cursorPosition=x.first;
            buffer=buffer.substr(0,x.first)+buffer.substr(x.second);
            //cursorPosition-=(x.second-x.first);
            buffer=buffer.substr(0,cursorPosition)+clipboard+buffer.substr(cursorPosition);
            cursorPosition+=clipboard.size();
        }else{
            buffer=buffer.substr(0,cursorPosition)+clipboard+buffer.substr(cursorPosition);
            cursorPosition+=clipboard.size();
        }
        editor.UnselectText();
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitPasteTextCommand(*this);
    }
};

/* Привести выделенный текст в ВЕРХНИЙ регистр */
class UppercaseTextCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        if (editor.HasSelection()){
            std::pair<size_t,size_t> x = editor.GetSelection();
            for (size_t i = x.first; i < x.second; i++)
            {
                buffer[i]=toupper(buffer[i]);
            }
            
        }
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitUppercaseTextCommand(*this);
    }
};

/* Привести выделенный текст в нижний регистр */
class LowercaseTextCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        if (editor.HasSelection()){
            std::pair<size_t,size_t> x = editor.GetSelection();
            for (size_t i = x.first; i < x.second; i++)
            {
                buffer[i]=tolower(buffer[i]);
            }
            
        }
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitLowercaseTextCommand(*this);
    }
};

/* Перенести курсор в конец строки */
class MoveToEndCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        if (int(buffer.substr(cursorPosition).find('\n'))!=-1){ 
            cursorPosition+=buffer.substr(cursorPosition).find('\n');
        }else{
            cursorPosition=buffer.size();
        }
        editor.UnselectText();
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitMoveToEndCommand(*this);
    }
};

/* Перенести курсор в начало строки */
class MoveToStartCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        if (int(buffer.substr(0,cursorPosition).rfind('\n'))!=-1){ 
            cursorPosition=buffer.substr(0,cursorPosition).rfind('\n')+1;
        }else{
            cursorPosition=0;
        }
        editor.UnselectText();
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitMoveToStartCommand(*this);
    }
};

/* Удалить часть строки, начиная от позиции курсора до первого пробела или конца строки */
class DeleteWordCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        size_t m;
        /*
        if (int(buffer.substr(cursorPosition).find(' '))!=-1 && int(buffer.substr(cursorPosition).find('\n'))!=-1)
        {
            m = std::min(buffer.substr(cursorPosition).find(' '), buffer.substr(cursorPosition).find('\n'))+1;
        }else if (int(buffer.substr(cursorPosition).find(' '))!=-1){
            m=buffer.substr(cursorPosition).find(' ')+1;
        }else if (int(buffer.substr(cursorPosition).find('\n'))!=-1){
            m=buffer.substr(cursorPosition).find('\n');
        }else{
            m=buffer.size()-cursorPosition;
        }
        */
        //std::cout << cursorPosition << "    " << buffer.substr(cursorPosition).find(' ') << std:: endl;
        m=std::min(buffer.substr(cursorPosition).find(' '), buffer.substr(cursorPosition).find('\n'));
        buffer=buffer.substr(0,cursorPosition)+buffer.substr(m+cursorPosition);
        editor.UnselectText();
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitDeleteWordCommand(*this);
    }
};

/* Макрос */
class MacroCommand : public ICommand {
public:
    MacroCommand(std::list<std::shared_ptr<ICommand>>& vec): vec_(vec) {}
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        for (auto x : vec_)
        {
            x->Apply(buffer,cursorPosition,clipboard,editor);
        }
    }
    void AcceptVisitor(CommandVisitor& visitor) override {}
private:
    std::list<std::shared_ptr<ICommand>> vec_;
};

CommandBuilder::CommandBuilder()
{
    selectionSize_=0;
    text_="";
    subcommands_={};
    logStreamPtr_=nullptr;
}

CommandBuilder& CommandBuilder::WithType(Type type)
{
    type_=type;
    return *this;
}

CommandBuilder& CommandBuilder::SelectionSize(size_t selectionSize)
{
    selectionSize_=selectionSize;
    return *this;
}

CommandBuilder& CommandBuilder::Text(std::string text)
{
    text_=text;
    return *this;
}

CommandBuilder& CommandBuilder::LogTo(std::ostream& logStream)
{
    logStreamPtr_=&logStream;
    return *this;
}

CommandBuilder& CommandBuilder::AddSubcommand(CommandPtr subcommand)
{
    //LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : (*logStreamPtr_)), subcommand);
    subcommands_.push_back(subcommand);
    return *this;
}

CommandPtr CommandBuilder::build()
{
    if (type_==CommandBuilder::Type::InsertText){
        InsertTextCommand cls(text_);
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : (*logStreamPtr_)), std::make_shared<InsertTextCommand>(cls));
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::MoveCursorLeft){
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<MoveCursorLeftCommand>());
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::MoveToEnd){
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<MoveToEndCommand>());
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::MoveCursorUp){
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<MoveCursorUpCommand>());
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::SelectText){
        SelectTextCommand cl1(selectionSize_);
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<SelectTextCommand>(cl1));
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::CopyText){
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<CopyTextCommand>());
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::PasteText){
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<PasteTextCommand>());
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::MoveToStart){
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<MoveToStartCommand>());
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::Macro){
        std::list<CommandPtr> mac;
        for (auto x : subcommands_)
        {
            LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), x);
            mac.push_back(std::make_shared<LoggedCommandWrapper>(cl));
        }
        MacroCommand mac1(mac);
        return std::make_shared<MacroCommand>(mac1);
    }else if (type_==CommandBuilder::Type::DeleteWord){
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<DeleteWordCommand>());
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::MoveCursorDown){
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<MoveCursorDownCommand>());
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::MoveCursorRight){
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<MoveCursorRightCommand>());
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::DeleteText){
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<DeleteTextCommand>());
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::UppercaseText){
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<UppercaseTextCommand>());
        return std::make_shared<LoggedCommandWrapper>(cl);
    }else if (type_==CommandBuilder::Type::LowercaseText){
        LoggedCommandWrapper cl((logStreamPtr_==nullptr ? std::cout : *logStreamPtr_), std::make_shared<LowercaseTextCommand>());
        return std::make_shared<LoggedCommandWrapper>(cl);
    }

    return std::make_shared<MoveCursorLeftCommand>();
}

