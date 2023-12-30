package com.xormedia.aqua.sdk;

import java.util.*;
import java.io.*;

/**
 * A simple integer variable that allows to pass back the integer value out of a function call via a parameter.<p>
 * @author hui.shao
 */
public class LongVariable {
	
	/**
	 * Constructor
	 * @param val the initial value of the variable
	 */
	public LongVariable(long val) {
		set(val);
	}

	/**
	 * Set the value of the variable
	 * @param val the new integer value is about to set to
	 * @return the integer value has just been set
	 */
	public long set(long val) {
		_val = val;
		return get();
	}

	/**
	 * Get the value of the variable
	 * @return the integer value of variable
	 */
	public long get() {
		return _val;
	}

	private long _val;
}