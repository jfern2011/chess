#ifndef __OUTPUTS_H__
#define __OUTPUTS_H__

#include "chess.h"
#include "Signal.h"

/**
 * The base class for \ref OutputElement objects
 */
class element_base
{

public:

	element_base( const std::string& name );

	virtual ~element_base();

	std::string get_name() const;

	virtual bool has_updated() const = 0;

	virtual void mark_stale() = 0;

	virtual bool update() = 0;

protected:

	/**
	 * True if \ref update() changed our value
	 */
	bool _has_changed;

	/**
	 *  The name of this element
	 */
	const std::string
		_name;
};

/**
 * @class OutputElement
 *
 * Represents a single output element from the search algorithm, which
 * can be combined with others to send groups of outputs to the GUI
 * on the same line. Examples of output elements include the principal
 * variation, the search depth, the score, and so on
 *
 * @tparam T The type of element, which needs to be convertible to
 *           a string via \ref Util::to_string()
 */
template <typename T>
class OutputElement : public element_base
{

public:

	/**
	 * Constructor
	 *
	 * @param[in] name This element's name
	 */
	OutputElement(const std::string& name)
		: element_base(name),
		  _updater(), _value()
	{
	}

	/**
	 * Destructor
	 */
	~OutputElement()
	{
	}

	/**
	 * Assign the function from which to grab new values for this
	 * element, e.g. new val = func()
	 *
	 * @tparam C The class that implements the update function
	 *
	 * @param[in] obj  An object through which to call the update
	 *                 function
	 * @param[in] func The updater function
	 *
	 * @return True on success
	 */
	template <typename C>
	bool assign_updater(C& obj, T(C::*func)())
	{
		AbortIfNot(_updater.attach(obj, func),
			false);

		return true;
	}

	/**
	 * Assign the function from which to grab new values for this
	 * element, e.g. new val = func()
	 *
	 * @tparam C The class that implements the update function
	 *
	 * @param[in] obj  An object through which to call the update
	 *                 function
	 * @param[in] func The *const* updater function
	 *
	 * @return True on success
	 */
	template <typename C>
	bool assign_updater(C& obj, T(C::*func)() const)
	{
		AbortIfNot(_updater.attach(obj, func),
			false);

		return true;
	}

	/**
	 * Get the current value of this element
	 *
	 * @return The element's value
	 */
	T& get()
	{
		return _value;
	}

	/**
	 * Check if our value has changed since the last \ref get()
	 *
	 * @return True if the element was updated
	 */
	bool has_updated() const
	{
		return _has_changed;
	}

	/**
	 * Mark this element as stale, indicating we should run
	 * \ref update() to refresh it
	 */
	void mark_stale()
	{
		_has_changed = false;
	}

	/**
	 * Invoke the updater function, updating the current value of
	 * this element ONLY if it has changed
	 *
	 * @return True on success
	 */
	bool update()
	{
		AbortIfNot(_updater.is_connected(),
			false);

		T temp = _updater.raise();

		if (temp != _value)
		{
			_has_changed  =  true;
			_value = temp;
		}

		return true;
	}

private:

	/**
	 * Invokes the updater function
	 */
	Signal::Signal<T>
		_updater;

	/**
	 * This element's current value
	 */
	T _value;
};

/**
 * @class EngineOutputs
 *
 * Maintains a record of outputs (e.g. search stats) that get sent to
 * the GUI. Once all output elements are created, every call to
 * \ref update() will iterate through all elements and update each of
 * them. Since this is linear in time, \ref update() should be
 * called seldomly
 */
class EngineOutputs
{

public:

	EngineOutputs();

	~EngineOutputs();

	/**
	 * Create a new output element
	 *
	 * @tparam T  The data type of this element
	 * @tparam C  The class that implements the function that updates
	 *            this element
	 *
	 * @param[in] _name The name of this element
	 * @param[in] obj   The object through which to invoke the update
	 *                  function
	 * @param[in] func  The update function
	 *
	 * @return A unique element id
	 */
	template <typename T, typename C>
	int create(const std::string& _name, C& obj, T(C::*func)())
	{
		auto element = _create_common<T>(_name);

		AbortIfNot(element->assign_updater(obj, func), -1);

		const int id = _elements.size();

		_elements.push_back(element);
		return id;
	}

	/**
	 * Create a new output element
	 *
	 * @tparam T  The data type of this element
	 * @tparam C  The class that implements the function that updates
	 *            this element
	 *
	 * @param[in] _name The name of this element
	 * @param[in] obj   The object through which to invoke the update
	 *                  function
	 * @param[in] func  The *const* update function
	 *
	 * @return A unique element id
	 */
	template <typename T, typename C>
	int create(const std::string& _name, C& obj, T(C::*func)() const)
	{
		auto element = _create_common<T>(_name);

		AbortIfNot(element->assign_updater(obj, func), -1);

		const int id = _elements.size();

		_elements.push_back(element);
		return id;
	}

	/**
	 * Get the output element at the specified index
	 *
	 * @tparam The data storage type
	 *
	 * @param[in] index Get the element located here
	 * @param[out] value The element's value
	 *
	 * @return True on success
	 */
	template < typename T > bool get( size_t index, T& value )
	{
		AbortIf(size() <= index, false);

		auto element =
			dynamic_cast<OutputElement<T>*>(_elements[index]);

		element->mark_stale();

		value = element->get();
		return true;
	}

	int get_id(const std::string& name) const;

	size_t size() const;

	void update();

	const element_base*
		operator[]( const std::string& name ) const;

	const element_base*
		operator[](size_t index) const;

private:

	/**
	 * Common element creation
	 *
	 * @param[in] _name The name of the element
	 *
	 * @return The newly created element
	 */
	template <typename T>
	OutputElement<T>* _create_common(const std::string& _name)
	{
		const std::string name = Util::trim(_name);
		AbortIf(name.empty(), nullptr);

		return new OutputElement<T>(name);
	}

	/**
	 * All of our output elements
	 */
	std::vector<element_base*>
		_elements;
};

/*
class EngineOutputs
{

public:

	typedef struct
	{
		int  cp;
		int  mate;
		bool lowerbound;
		bool upperbound;

	} score_t;

	EngineOutputs();

	~EngineOutputs();

	int bestmove;

	int cpuload;

	// The current line being evaluated (not yet implemented)
	Buffer<int,MAX_PLY> currline;

	int currmove;

	int currmovenumber;

	// The current search depth, in plies
	int depth;

	int hashfull;

	int multipv;

	int64 nodes;

	int nps;

	int pondermove;

	Buffer<int,MAX_PV,MAX_PLY>
		pv;

	// The refutation line per move (not yet implemented)
	Buffer<int,MAX_MOVES,MAX_PLY>
		refutation;

	score_t score;

	// The current selective search depth (not yet implemented)
	int seldepth;

	std::string string;

	int tbhits;

	int time;
};
*/

#endif
