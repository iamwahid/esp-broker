// #ifndef _EVENT_LOGIC_H_
// #define _EVENT_LOGIC_H_

// #include <map>
// #include <memory>
// #include <vector>
// #include <string.h>
// #include "EspNowMQ.h"
// #include "../globalConf.h"

// using namespace std;

// //predefine to prevent some linker problems
// class L_VAR;
// //


// //This is the base class for all PLC ladder logic objects. Individual object types derive from this class.
// class L_OBJ
// {
// public:
// 	L_OBJ( const String &evt_name, OBJ_TYPE type ){ l_event = evt_name; l_type = type; l_state = 0; }
// 	virtual ~L_OBJ(){ }
// 	//Sets the state of the L_OBJ
// 	void setState(uint8_t state) { l_state = state; }
// 	//Returns enabled/disabled/etc
// 	uint8_t getState(){ return l_state; } 
// 	//Returns the object type identifier (OUTPUT/INPUT/TIMER,etc.)
// 	const OBJ_TYPE getType(){ return l_type; }
// 	//Returns the unique object ID
// 	const String &getID(){ return l_event; }

// 	//Returns an object's bit (L_VAR pointer) based on an inputted bit ID string
// 	virtual shared_ptr<L_VAR> getObjectVAR( const String &event_name );
// 	//This function returns a reference to the object's local variable storage container (this may or may not be used, depending on the object's type).
// 	vector<shared_ptr<L_VAR>> &getObjectVARs(){ return localVars; }

// private:
// 	OBJ_TYPE l_type; //Identifies the type of this object. 0 = input, 1 = Physical output, 2 = Virtual Output, 3 = timer, etc.	
// 	uint8_t l_state; //Enabled or disabled?
// 	String l_event; //The unique ID for this object (globally)

// 	vector<shared_ptr<L_VAR>> localVars; //locally stored ladder var objects (that belong to this object)
// };

// //L_OBJ_Logical objects are a subclass of L_OBJ. These are objects that are used in performing logic operations via the logic script. 
// class L_OBJ_Logical : public L_OBJ
// {
// 	public:
// 	L_OBJ_Logical( const String &event_name, OBJ_TYPE type ) : L_OBJ( event_name, type ) {}
// 	~L_OBJ_Logical(){}
// 	virtual void setLineState(bool &state, bool bNot){ if (state) b_lineState = state; } //save the state. Possibly consider latching the state if state is HIGH (duplicate outputs?)
// 	//Returns the currently stored line state for the given object.
// 	bool getLineState(){ return b_lineState; }
// 	//Returns the logic type of the object. EX: Normally Open, Normally closed, etc.
// 	const uint8_t getLogic() { return i_objLogic; }

// 	//Sets the logic type for the given object. EX: Normally Open, Normally closed, etc.
// 	void setLogic(uint8_t logic) { i_objLogic = logic; }
// 	//Set the line state back to false for the next scan This should only be called by the rung manager (which applies the logic after processing)
// 	virtual void updateObject(){ b_lineState = false; } 

// 	private:
// 	uint8_t i_objLogic;
// 	bool b_lineState;
// };


// //This object serves as a means of storing logic script specific flags that pertain to a single ladder object. 
// //This allows us to perform multiple varying logic operations without the need to create multiple copies of the same object.
// struct L_OBJ_Wrapper 
// {
// 	L_OBJ_Wrapper(shared_ptr<L_OBJ_Logical> obj, uint16_t rung, bool not_flag = false)
// 	{
// 		bNot = not_flag; //Exclusively for NOT logic
// 		ladderOBJ = obj; 
// 		i_rungNum = rung; //store this here for now
// 	}
// 	~L_OBJ_Wrapper(){ }

// 	//Adds the inputted object to the current object's list.
// 	bool addNextObject( shared_ptr<L_OBJ_Wrapper> pObj )
// 	{
// 		nextObjects.push_back(pObj);
// 		return true; 
// 	}
// 	bool addNextObject( const vector<shared_ptr<L_OBJ_Wrapper>> &pObj )
// 	{
// 		for ( uint8_t x = 0; x < pObj.size(); x++ )
// 			nextObjects.push_back(pObj[x]);

// 		return true; 
// 	}

// 	void setLineState(bool state)
// 	{
// 		getObject()->setLineState(state, getNot()); //line state needs to be set per wrapper, not per ladder object
		
// 		for(uint8_t x = 0; x < nextObjects.size(); x++ )
// 			nextObjects[x]->setLineState(state);
// 	}
	
// 	//Returns the pointer to the ladder object stored by this object.
// 	const shared_ptr<L_OBJ_Logical> getObject(){ return ladderOBJ; }
// 	//Tells us if the object is being interpreted using NOT logic
// 	bool getNot(){ return bNot; }
		
// 	private:
// 	bool bNot; //if the object is using not logic (per instance in rungs)
// 	uint16_t i_rungNum;
// 	shared_ptr<L_OBJ_Logical> ladderOBJ; //Container for the actual L_Obj object
// 	vector<shared_ptr<L_OBJ_Wrapper>> nextObjects;
// };

// class L_VAR : public L_OBJ_Logical
// {
// 	public:
// 	//These constructors are for pointers to existing variables
// 	L_VAR( shared_ptr<L_VAR> var, const String &event_name ) : L_OBJ_Logical( event_name, var->getType() ){ values = var->values; b_usesPtr = var->b_usesPtr; }  
// 	L_VAR( int_fast32_t *value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_INT ){ values.i.val_ptr = value; b_usesPtr = true; }
// 	L_VAR( uint_fast32_t *value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_UINT ){ values.ui.val_ptr = value; b_usesPtr = true; }
// 	L_VAR( bool *value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_BOOL ){ values.b.val_ptr = value; b_usesPtr = true; }
// 	L_VAR( uint16_t *value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_USHORT ){ values.us.val_ptr = value; b_usesPtr = true; }
// 	L_VAR( double *value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_FLOAT ){ values.d.val_ptr = value; b_usesPtr = true; }
// 	L_VAR( uint64_t *value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_ULONG ){ values.ul.val_ptr = value; b_usesPtr = true; }
// 	L_VAR( int64_t *value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_LONG ){ values.l.val_ptr = value; b_usesPtr = true; }
// 	//
// 	//These constructors are for locally stored values
// 	L_VAR( int_fast32_t value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_INT ){ values.i.val = value; b_usesPtr = false; }
// 	L_VAR( uint_fast32_t value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_UINT ){ values.ui.val = value; b_usesPtr = false; }
// 	L_VAR( bool value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_BOOL ){ values.b.val = value; b_usesPtr = false; }
// 	L_VAR( uint16_t value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_USHORT ){ values.us.val = value; b_usesPtr = false; }
// 	L_VAR( double value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_FLOAT ){ values.d.val = value; b_usesPtr = false; }
// 	L_VAR( uint64_t value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_ULONG ){ values.ul.val = value; b_usesPtr = false; }
// 	L_VAR( int64_t value, const String &event_name ) : L_OBJ_Logical( event_name, OBJ_TYPE::TYPE_VAR_LONG ){ values.l.val = value; b_usesPtr = false; }
// 	//
// 	virtual void updateObject()
// 	{ 
// 		L_OBJ_Logical::updateObject(); 
// 	}

// 	//this function returns a string that represents the currently stored value in the variable object.
// 	String getValueStr();

// 	bool operator>(const L_VAR &);
// 	bool operator>=(const L_VAR &);
// 	bool operator<(const L_VAR &);
// 	bool operator<=(const L_VAR &);
// 	bool operator==(const L_VAR &);
// 	bool operator!=(const L_VAR &);
// 	void operator=(const L_VAR &);

// 	template <class T>
// 	T getValue()
// 	{
// 			switch(getType())
// 			{
// 					case OBJ_TYPE::TYPE_VAR_BOOL:
// 					{
// 							if ( b_usesPtr )
// 									return static_cast<T>(*values.b.val_ptr);
// 							else
// 									return static_cast<T>(values.b.val);
// 					}
// 					break;
// 					case OBJ_TYPE::TYPE_VAR_FLOAT:
// 					{
// 							if ( b_usesPtr )
// 									return static_cast<T>(*values.d.val_ptr);
// 							else
// 									return static_cast<T>(values.d.val);
// 					}
// 					break;
// 					case OBJ_TYPE::TYPE_VAR_USHORT:
// 					{
// 							if ( b_usesPtr )
// 									return static_cast<T>(*values.us.val_ptr);
// 							else
// 									return static_cast<T>(values.us.val);
// 					}
// 					break;
// 					case OBJ_TYPE::TYPE_VAR_INT:
// 					{
// 							if ( b_usesPtr )
// 									return static_cast<T>(*values.i.val_ptr);
// 							else
// 									return static_cast<T>(values.i.val);
// 					}
// 					break;
// 					case OBJ_TYPE::TYPE_VAR_UINT: 
// 					{
// 							if ( b_usesPtr )
// 									return static_cast<T>(*values.ui.val_ptr);
// 							else
// 									return static_cast<T>(values.ui.val);
// 					}
// 					break;
// 					case OBJ_TYPE::TYPE_VAR_LONG:
// 					{
// 							if ( b_usesPtr )
// 									return static_cast<T>(*values.l.val_ptr);
// 							else
// 									return static_cast<T>(values.l.val);
// 					}
// 					break;
// 					case OBJ_TYPE::TYPE_VAR_ULONG:
// 					{
// 							if ( b_usesPtr )
// 									return static_cast<T>(*values.ul.val_ptr);
// 							else
// 									return static_cast<T>(values.ul.val);
// 					}
// 					break;
// 					default:
// 					break;
// 			}

// 			return static_cast<T>(0);
// 	}

// 	template <typename T>
// 	void setValue( const T val ) //Doesn't support String type
// 	{
// 			switch(getType())
// 			{
// 					case OBJ_TYPE::TYPE_VAR_BOOL:
// 					{
// 							if ( b_usesPtr )
// 									*values.b.val_ptr = static_cast<bool>(val);
// 							else
// 									values.b.val = static_cast<bool>(val);
// 					}
// 					break;
// 					case OBJ_TYPE::TYPE_VAR_FLOAT:
// 					{
// 							if ( b_usesPtr )
// 									*values.d.val_ptr = static_cast<double>(val);
// 							else
// 									values.d.val = static_cast<double>(val);
// 					}
// 					break;
// 					case OBJ_TYPE::TYPE_VAR_USHORT:
// 					{
// 							if ( b_usesPtr )
// 									*values.us.val_ptr = static_cast<uint16_t>(val);
// 							else
// 									values.us.val = static_cast<uint16_t>(val);
// 					}
// 					break;
// 					case OBJ_TYPE::TYPE_VAR_INT:
// 					{
// 							if ( b_usesPtr )
// 									*values.i.val_ptr = static_cast<int_fast32_t>(val);
// 							else
// 									values.i.val = static_cast<int_fast32_t>(val);
// 					}
// 					break;
// 					case OBJ_TYPE::TYPE_VAR_UINT:
// 					{
// 							if ( b_usesPtr )
// 									*values.ui.val_ptr = static_cast<uint_fast32_t>(val);
// 							else
// 									values.ui.val = static_cast<uint_fast32_t>(val);
// 					}
// 					break;
// 					case OBJ_TYPE::TYPE_VAR_LONG:
// 					{
// 							if ( b_usesPtr )
// 									*values.l.val_ptr = static_cast<int64_t>(val);
// 							else
// 									values.l.val = static_cast<int64_t>(val);
// 					}
// 					break;
// 					case OBJ_TYPE::TYPE_VAR_ULONG:
// 					{
// 							if ( b_usesPtr )
// 									*values.ul.val_ptr = static_cast<uint64_t>(val);
// 							else
// 									values.ul.val = static_cast<uint64_t>(val);
// 					}
// 					break;
// 					default:
// 					break;
// 			}
// 	}
// 	void setValue( const String & );

// 	virtual void setLineState(bool &, bool);

// 	private:
// 	template <typename T>
// 	union group
// 	{
// 		T *val_ptr;
// 		T val;
// 	};
// 	union
// 	{
// 		group<int64_t> l;
// 		group<uint64_t> ul;
// 		group<int_fast32_t> i;
// 		group<uint_fast32_t> ui;
// 		group<uint16_t> us;
// 		group<double> d;
// 		group<bool> b;
// 	} values;

// 	bool b_usesPtr; //tells us if we're using a pointer to an object of the same type, or if we're using a locally stored value.
// };


// #endif

