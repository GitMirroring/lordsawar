/* Defines String::ucompose(fmt, arg...) for easy, i18n-friendly
 * composition of strings with Gtkmm >= 1.3.* (see www.gtkmm.org).
 * Uses Glib::ustring instead of std::string which doesn't work with
 * Gtkmm due to character encoding troubles with stringstreams.
 *
 * Version 1.0.4.
 *
 * Copyright (c) 2002, 03, 04 Ole Laursen <olau@hardworking.dk>.
 * Copyright (C) 2015, 2017, 2021 Ben Asselstine
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
 * 02110-1301, USA.
 */

//
// Basic usage is like
//
//   String::ucompose("This is a %1x%2 matrix.", rows, cols);
//
// See http://www.cs.aau.dk/~olau/compose/ or the included
// README.compose for more details.
//

/*
 * the ucompose routines have been neutred to use Glib::ustring::ucompose.
 * they seem to work fine.
 */
#ifndef STRING_UCOMPOSE_HPP
#define STRING_UCOMPOSE_HPP

#include <glibmm/ustring.h>
#include <glibmm/convert.h>

#include <sstream>
#include <string>
#include <list>
#include <map>			// for multimap

namespace UStringPrivate
{
  // the actual composition class - using String::ucompose is cleaner, so we
  // hide it here
  class Composition
  {
  public:
    // initialize and prepare format string on the form "text %1 text %2 etc."
    explicit Composition(std::string fmt);

    // supply an replacement argument starting from %1
    template <typename T>
    Composition &arg(const T &obj);

    // compose and return string
    Glib::ustring str() const;

  private:
    std::wostringstream os;
    int arg_no;

    // we store the output as a list - when the output string is requested, the
    // list is concatenated to a string; this way we can keep iterators into
    // the list instead of into a string where they're possibly invalidated
    // when inserting a specification string
    typedef std::list<std::string> output_list;
    output_list output;

    // the initial parse of the format string fills in the specification map
    // with positions for each of the various %?s
    typedef std::multimap<int, output_list::iterator> specification_map;
    specification_map specs;

    template <typename T>
    std::string stringify(T obj);
  };

  // helper for converting spec string numbers
  inline int char_to_int(char c)
  {
    switch (c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    default: return -1000;
    }
  }

  inline bool is_number(int n)
  {
    switch (n) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      return true;
    
    default:
      return false;
    }
  }

  template <typename T>
  inline std::string Composition::stringify(T obj)
  {
    os << obj;

    std::wstring s = os.str();
    
    return Glib::convert(std::string(reinterpret_cast<const char *>(s.data()),
				     s.size() * sizeof(wchar_t)),
			 "UTF-8", "WCHAR_T");
  }

  // specialisations for the common string types
  template <>
  inline std::string
  Composition::stringify<std::string>(std::string obj)
  {
    return obj;
  }
  
  template <>
  inline std::string
  Composition::stringify<Glib::ustring>(Glib::ustring obj)
  {
    return obj;
  }
  
  template <>
  inline std::string
  Composition::stringify<const char *>(const char *obj)
  {
    return obj;
  }
  
  // implementation of class Composition
  template <typename T>
  inline Composition &Composition::arg(const T &obj)
  {
    Glib::ustring rep = stringify(obj);
    
    if (!rep.empty()) {		// manipulators don't produce output
      for (specification_map::const_iterator i = specs.lower_bound(arg_no),
	     end = specs.upper_bound(arg_no); i != end; ++i) {
	output_list::iterator pos = i->second;
	++pos;
      
	output.insert(pos, rep);
      }
    
      os.str(std::wstring());
      //os.clear();
      ++arg_no;
    }
  
    return *this;
  }

  inline Composition::Composition(std::string fmt)
    : arg_no(1)
  {
//#if __GNUC__ >= 3
    //os.imbue(std::locale("")); // use the user's locale for the stream
//#endif
    std::string::size_type b = 0, i = 0;
  
    // fill in output with the strings between the %1 %2 %3 etc. and
    // fill in specs with the positions
    while (i < fmt.length()) {
      if (fmt[i] == '%' && i + 1 < fmt.length()) {
	if (fmt[i + 1] == '%') { // catch %%
	  fmt.replace(i, 2, "%");
	  ++i;
	}
	else if (is_number(fmt[i + 1])) { // aha! a spec!
	  // save string
	  output.push_back(fmt.substr(b, i - b));
	
	  int n = 1;		// number of digits
	  int spec_no = 0;

	  do {
	    spec_no += char_to_int(fmt[i + n]);
	    spec_no *= 10;
	    ++n;
	  } while (i + n < fmt.length() && is_number(fmt[i + n]));

	  spec_no /= 10;
	  output_list::iterator pos = output.end();
	  --pos;		// safe since we have just inserted a string
	
	  specs.insert(specification_map::value_type(spec_no, pos));
	
	  // jump over spec string
	  i += n;
	  b = i;
	}
	else
	  ++i;
      }
      else
	++i;
    }
  
    if (i - b > 0)		// add the rest of the string
      output.push_back(fmt.substr(b, i - b));
  }

  inline Glib::ustring Composition::str() const
  {
    // assemble string
    std::string s;
  
    for (output_list::const_iterator i = output.begin(), end = output.end();
	 i != end; ++i)
      s += *i;
  
    return s;
  }
}


namespace String 
{
  // a series of functions which accept a format string on the form "text %1
  // more %2 less %3" and a number of templated parameters and spits out the
  // composited string
  template <typename T1>
  inline Glib::ustring ucompose(const Glib::ustring &fmt, const T1 &o1)
  {
    return Glib::ustring::compose(fmt, o1);
  }

  template <typename T1, typename T2>
  inline Glib::ustring ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2)
  {
    return Glib::ustring::compose(fmt, o1, o2);
  }

  template <typename T1, typename T2, typename T3>
  inline Glib::ustring ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3)
  {
    return Glib::ustring::compose(fmt, o1, o2, o3);
  }

  template <typename T1, typename T2, typename T3, typename T4>
  inline Glib::ustring ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4)
  {
    return Glib::ustring::compose(fmt, o1, o2, o3, o4);
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5>
  inline Glib::ustring ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5)
  {
    return Glib::ustring::compose(fmt, o1, o2, o3, o4, o5);
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5,
	    typename T6>
  inline Glib::ustring ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5, const T6 &o6)
  {
    return Glib::ustring::compose(fmt, o1, o2, o3, o4, o5, o6);
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5,
	    typename T6, typename T7>
  inline Glib::ustring ucompose(const Glib::ustring &fmt,
				const T1 &o1, const T2 &o2, const T3 &o3,
				const T4 &o4, const T5 &o5, const T6 &o6,
				const T7 &o7)
  {
    return Glib::ustring::compose(fmt, o1, o2, o3, o4, o5, o6, o7);
  }

  inline Glib::ustring utrim(Glib::ustring str) 
  {
    Glib::ustring white = " \n\t\r\v\a\b\f";

    if(str.empty () == true)
      return str;

    Glib::ustring::size_type start = str.find_first_not_of (white);
    Glib::ustring::size_type finish = str.find_last_not_of (white);

    if(start == Glib::ustring::npos)
      return "";

    return Glib::ustring (str, start, finish - start + 1);
  }

  inline Glib::ustring capitalize(Glib::ustring str)
  {
    Glib::ustring u = str.uppercase();
    Glib::ustring out = u.substr(0, 1);
    out += str.substr (1);
    return out;
  }

  inline Glib::ustring strip_trailing_numbers (Glib::ustring name)
    {
      int digits = 0;
      for (Glib::ustring::reverse_iterator i = name.rbegin ();
                      i != name.rend(); ++i)
        {
          if (g_unichar_isdigit (*i))
            digits++;
          else
            break;
        }
      return name.substr (0, name.length () - digits);
    }

  inline Glib::ustring indent (Glib::ustring s, int num_spaces)
    {
      Glib::ustring result = "";
      std::string delimiter = "\n";
      std::string spaces = "";
      for (int i = 0; i < num_spaces; i++)
        spaces += " ";
      Glib::ustring::size_type idx = s.rfind(delimiter);
      if (idx == Glib::ustring::npos ||
          idx != s.length() - delimiter.length())
        s += delimiter;
      size_t pos = 0;
      std::string token;
      while ((pos = s.find (delimiter)) != std::string::npos)
        {
          token = s.substr (0, pos);
          result += spaces + token + delimiter;
          s.erase (0, pos + delimiter.length ());
        }
      return result;
    }
}

#endif // STRING_UCOMPOSE_HPP
