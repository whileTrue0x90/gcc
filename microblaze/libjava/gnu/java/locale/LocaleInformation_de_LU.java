/* LocaleInformation_de_LU.java
   Copyright (C) 2002 Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */


// This file was automatically generated by localedef.

package gnu.java.locale;

import java.util.ListResourceBundle;

public class LocaleInformation_de_LU extends ListResourceBundle
{
  static final String decimalSeparator = LocaleInformation_de_DE.decimalSeparator;
  static final String groupingSeparator = LocaleInformation_de_DE.groupingSeparator;
  static final String numberFormat = LocaleInformation_de_DE.numberFormat;
  static final String percentFormat = LocaleInformation_de_DE.percentFormat;
  static final String[] weekdays = { null, "Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag" };

  static final String[] shortWeekdays = { null, "Son", "Mon", "Die", "Mit", "Don", "Fre", "Sam" };

  static final String[] shortMonths = { "Jan", "Feb", "M\u00E4r", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez", null };

  static final String[] months = { "Januar", "Februar", "M\u00E4rz", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember", null };

  static final String[] ampms = { "", "" };

  static final String shortDateFormat = "yyyy-MM-dd";
  static final String defaultTimeFormat = "";
  static final String currencySymbol = "FLUX";
  static final String intlCurrencySymbol = "LUF";
  static final String currencyFormat = "$ #,###,##0.00;$ -#,###,##0.00";

  private static final Object[][] contents =
  {
    { "weekdays", weekdays },
    { "shortWeekdays", shortWeekdays },
    { "shortMonths", shortMonths },
    { "months", months },
    { "ampms", ampms },
    { "shortDateFormat", shortDateFormat },
    { "defaultTimeFormat", defaultTimeFormat },
    { "currencySymbol", currencySymbol },
    { "intlCurrencySymbol", intlCurrencySymbol },
    { "currencyFormat", currencyFormat },
    { "decimalSeparator", decimalSeparator },
    { "groupingSeparator", groupingSeparator },
    { "numberFormat", numberFormat },
    { "percentFormat", percentFormat },
  };

  public Object[][] getContents () { return contents; }
}
