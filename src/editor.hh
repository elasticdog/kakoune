#ifndef editor_hh_INCLUDED
#define editor_hh_INCLUDED

#include "buffer.hh"
#include "selection.hh"
#include "filter.hh"
#include "idvaluemap.hh"
#include "hooks_manager.hh"

namespace Kakoune
{

class IncrementalInserter;

// An Editor is a buffer mutator
//
// The Editor class provides methods to manipulate a set of selections
// and to use these selections to mutate it's buffer.
class Editor
{
public:
    typedef BufferString String;
    typedef std::function<Selection (const BufferIterator&)> Selector;
    typedef std::function<SelectionList (const Selection&)>  MultiSelector;

    Editor(Buffer& buffer);
    virtual ~Editor() {}

    Buffer& buffer() const { return m_buffer; }

    void erase();
    void insert(const String& string);
    void append(const String& string);
    void replace(const String& string);

    void push_selections();
    void pop_selections();

    void move_selections(const BufferCoord& offset, bool append = false);
    void clear_selections();
    void keep_selection(int index);
    void select(const BufferIterator& iterator);
    void select(const Selector& selector, bool append = false);
    void multi_select(const MultiSelector& selector);

    BufferString selection_content() const;
    const SelectionList& selections() const { return m_selections.back(); }

    bool undo();
    bool redo();

    void add_filter(FilterAndId&& filter);
    void remove_filter(const std::string& id);

    CandidateList complete_filterid(const std::string& prefix,
                                    size_t cursor_pos = std::string::npos);

    bool is_inserting() const { return m_current_inserter != nullptr; }

private:
    void erase_noundo();
    void insert_noundo(const String& string);
    void append_noundo(const String& string);

    SelectionList& selections() { return m_selections.back(); }

    void check_invariant() const;

    friend class IncrementalInserter;
    IncrementalInserter* m_current_inserter;

    void begin_incremental_insert(IncrementalInserter* inserter);
    void end_incremental_insert(IncrementalInserter* inserter);
    virtual void on_begin_incremental_insert() {}
    virtual void on_end_incremental_insert() {}


    Buffer&                             m_buffer;
    std::vector<SelectionList>          m_selections;
    idvaluemap<std::string, FilterFunc> m_filters;
};

// An IncrementalInserter manage insert mode
class IncrementalInserter
{
public:
    enum class Mode
    {
        Insert,
        Append,
        Change,
        InsertAtLineBegin,
        AppendAtLineEnd,
        OpenLineBelow,
        OpenLineAbove
    };

    IncrementalInserter(Editor& editor, Mode mode = Mode::Insert);
    ~IncrementalInserter();

    void insert(const Editor::String& string);
    void insert_capture(size_t index);
    void erase();
    void move_cursors(const BufferCoord& offset);

    Buffer& buffer() const { return m_editor.buffer(); }

private:
    void apply(Modification&& modification) const;

    Editor& m_editor;
};

}

#endif // editor_hh_INCLUDED

