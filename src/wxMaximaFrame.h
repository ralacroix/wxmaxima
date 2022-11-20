// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2004-2015 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//            (C) 2012 Doug Ilijev <doug.ilijev@gmail.com>
//            (C) 2014-2015 Gunter Königsmann <wxMaxima@physikbuch.de>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
//  SPDX-License-Identifier: GPL-2.0+

/*!\file 
  This file declares the class wxMaximaFrame

  wxMaximaFrame draws everything which can be seen
  surrounding the worksheet.
*/
#ifndef WXMAXIMAFRAME_H
#define WXMAXIMAFRAME_H

#include "precomp.h"
#include <wx/wx.h>

#include <wx/dirctrl.h>
#include <wx/filehistory.h>
#include <wx/listbox.h>
#include <wx/bmpbuttn.h>
#include <wx/arrstr.h>
#include <wx/aui/aui.h>
#include <wx/notifmsg.h>
#include <wx/wrapsizer.h>

#include "ScrollingGenWizPanel.h"
#include "Worksheet.h"
#include "HelpBrowser.h"
#include "RecentDocuments.h"
#include "Version.h"
#include "MainMenuBar.h"
#include "History.h"
#include "XmlInspector.h"
#include "StatusBar.h"
#include "LogPane.h"
#include "ButtonWrapSizer.h"
#include <list>


/*! The frame containing the menu and the sidebars
 */
class wxMaximaFrame : public wxFrame
{
public:
  wxMaximaFrame(wxWindow *parent, int id, wxLocale *locale, const wxString &title,
                const wxPoint &pos = wxDefaultPosition,
                const wxSize &size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE | wxSYSTEM_MENU | wxCAPTION, bool becomeLogTarget = true);

  /*! The destructor
   */
  virtual ~wxMaximaFrame();

  /*! Shows or hides the toolbar
    \param show
    - true:  Show the toolbar
    - false: Hide the toolbar
  */
  void ShowToolBar(bool show);

  /*! A list of all events the maxima frame can receive

    @{
    This list serves several purposes:
    - wxwidgets uses this list to tell us what kind of events it has to inform us about.
    - we use these events for inter process communication.\n
    For example the "evaluate this cell" menu is clicked by the enter (or the shift+enter,
    depending on what option is set in the configuration).
    - Thirdly his enum is used for assigning panels an ID that matches the ID of the event
    that toggles them which makes the handling of these IDs easier.
  */


  /*! Hide all panes
    
    This event is assigned an ID higher than the highest ID wxWidgets assigns to 
    its internal events in order to avoid ID clashes.
  */
  int menu_pane_hideall;
    /*! Both used as the "toggle the math pane" command and as the ID of the math pane

      Since this enum is also used for iterating over the panes it is vital 
      that this entry stays that of the first pane in this enum.
    */
    int menu_pane_math;
    int menu_pane_history;      //!< Both the "toggle the history pane" command and the history pane
    int menu_pane_structure;    //!< Both the "toggle the structure pane" command and the structure
    int menu_pane_xmlInspector; //!< Both the "toggle the xml monitor" command and the monitor pane
    int menu_pane_format;    //!< Both the "toggle the format pane" command and the format pane
    int menu_pane_greek;     //!< Both the "toggle the greek pane" command and the "greek" pane
    int menu_pane_unicode;   //!< Both the "toggle the unicode pane" command and the "unicode" pane
    int menu_pane_log;       //!< Both the "toggle the log pane" command and the "log" pane
    int menu_pane_variables; //!< Both the "toggle the variables pane" command and the "variables" pane
    int menu_pane_draw;      //!< Both the "toggle the draw pane" command for the "draw" pane
    int menu_pane_help;      //!< Both the "toggle the draw pane" command for the help browser
    int menu_pane_symbols;   //!< Both the "toggle the symbols pane" command for the "symbols" pane
    /*! Both used as the "toggle the stats pane" command and as the ID of the stats pane

      Since this enum is also used for iterating over the panes it is vital 
      that this entry stays that of the last pane in this enum.
    */
    int menu_pane_stats;
    int menu_pane_dockAll;
    int input_line_id;
    int refresh_id;
    int menu_batch_id;
    int menu_load_id;
    int menu_sconsole_id;
    int menu_interrupt_id;
    int menu_solve;
    int menu_solve_to_poly;
    int menu_solve_num;
    int menu_allroots;
    int menu_bfallroots;
    int menu_realroots;
    int menu_solve_lin;
    int menu_solve_algsys;
    int menu_eliminate;
    int menu_solve_ode;
    int menu_ivp_1;
    int menu_ivp_2;
    int menu_rk;
    int menu_bvp;
    int menu_genmatrix;
    int menu_gen_mat;
    int menu_gen_mat_lambda;
    int menu_enter_mat;
    int menu_csv2mat;
    int menu_mat2csv;
    int menu_csv2list;
    int menu_list2csv;
    int menu_matrix_row;
    int menu_matrix_col;
    int menu_matrix_row_list;
    int menu_matrix_col_list;
    int menu_submatrix;
    int menu_matrix_multiply;
    int menu_matrix_exponent;
    int menu_matrix_hadamard_product;
    int menu_matrix_hadamard_exponent;
    int menu_matrix_loadLapack;
    int menu_matrix_dgeev_eigenvaluesOnly;
    int menu_matrix_dgeev;
    int menu_matrix_zgeev_eigenvaluesOnly;
    int menu_matrix_zgeev;
    int menu_matrix_dgeqrf;
    int menu_matrix_dgesv;
    int menu_matrix_dgesvd;
    int menu_matrix_dgesvd_valuesOnly;
    int menu_matrix_dlange_max;
    int menu_matrix_dlange_one;
    int menu_matrix_dlange_inf;
    int menu_matrix_dlange_frobenius;
    int menu_matrix_zlange_max;
    int menu_matrix_zlange_one;
    int menu_matrix_zlange_inf;
    int menu_matrix_zlange_frobenius;
    int menu_matrix_zheev;
    int menu_invert_mat;
    int menu_cpoly;
    int menu_determinant;
    int menu_rank;
    int menu_eigen;
    int menu_eigvect;
    int menu_fun_def;
    int menu_gensym;
    int menu_adjoint_mat;
    int menu_transpose;
    int menu_map_mat;
    int menu_map;
    int menu_map_lambda;
    int menu_copymatrix;
    int menu_ratsimp;
    int menu_radsimp;
    int menu_scanmapfactor;
    int menu_factor;
    int menu_horner;
    int menu_collapse;
    int menu_optimize;
    int menu_mainvar;
    int menu_grind;
    int menu_gfactor;
    int menu_expand;
    int menu_expandwrt;
    int menu_expandwrt_denom;
    int menu_stringproc_setposition;
    int menu_stringproc_getposition;
    int menu_stringproc_flush_output;
    int menu_stringproc_flength;
    int menu_stringproc_close;
    int menu_stringproc_opena;
    int menu_stringproc_openr;
    int menu_stringproc_openw;
    int menu_stringproc_printf;
    int menu_stringproc_readline;
    int menu_stringproc_readchar;
    int menu_stringproc_readbyte;
    int menu_stringproc_writebyte;
    int menu_stringproc_charp;
    int menu_stringproc_alphacharp;
    int menu_stringproc_alphanumericp;
    int menu_stringproc_digitcharp;
    int menu_stringproc_constituent;
    int menu_stringproc_uppercasep;
    int menu_stringproc_lowercasep;
    int menu_stringproc_create_ascii;
    int menu_stringproc_cequal;
    int menu_stringproc_cequalignore;
    int menu_stringproc_clessp;
    int menu_stringproc_clesspignore;
    int menu_stringproc_cgreaterp;
    int menu_stringproc_cgreaterpignore;
    int menu_stringproc_sequal;
    int menu_stringproc_sequalignore;
    int menu_stringproc_ascii;
    int menu_stringproc_cint;
    int menu_stringproc_unicode;
    int menu_stringproc_unicode_to_utf8;
    int menu_stringproc_utf8_to_unicode;
    int menu_stringproc_charat;
    int menu_stringproc_charlist;
    int menu_stringproc_simplode;
    int menu_stringproc_sinsert;
    int menu_stringproc_eval_string;
    int menu_stringproc_parse_string;
    int menu_stringproc_scopy;
    int menu_stringproc_sdowncase;
    int menu_stringproc_slength;
    int menu_stringproc_smake;
    int menu_stringproc_smismatch;
    int menu_stringproc_split;
    int menu_stringproc_sposition;
    int menu_stringproc_sremove;
    int menu_stringproc_sremovefirst;
    int menu_stringproc_tokens;
    int menu_stringproc_ssearch;
    int menu_stringproc_ssort;
    int menu_stringproc_ssubstfirst;
    int menu_stringproc_strim;
    int menu_stringproc_striml;
    int menu_stringproc_strimr;
    int menu_stringproc_number_to_octets;
    int menu_stringproc_octets_to_number;
    int menu_stringproc_octets_to_string;
    int menu_stringproc_string_to_octets;
    int menu_sregex_load;
    int menu_sregex_regex_compile;
    int menu_sregex_regex_match_pos;
    int menu_sregex_regex_match;
    int menu_sregex_regex_split;
    int menu_sregex_subst_first;
    int menu_sregex_regex_subst;
    int menu_sregex_string_to_regex;
    int menu_opsyst_load;
    int menu_opsyst_chdir;
    int menu_opsyst_mkdir;
    int menu_opsyst_rmdir;
    int menu_opsyst_getcurrentdirectory;
    int menu_opsyst_copy_file;
    int menu_opsyst_rename_file;
    int menu_opsyst_delete_file;
    int menu_opsyst_getenv;
    int menu_opsyst_directory;
    int menu_opsyst_pathname_directory;
    int menu_opsyst_pathname_name;
    int menu_opsyst_pathname_type;
    int menu_scsimp;
    int menu_xthru;
    int menu_talg;
    int menu_tellrat;
    int menu_modulus;
    int menu_trigsimp;
    int menu_trigreduce;
    int menu_trigexpand;
    int menu_trigrat;
    int menu_rectform;
    int menu_polarform;
    int menu_demoivre;
    int menu_exponentialize;
    int menu_num_out;
    int menu_stringdisp;
    int menu_to_float;
    int menu_to_bfloat;
    int menu_to_numer;
    int menu_rat;
    int menu_rationalize;
    int menu_guess_exact_value;
    int menu_quad_qag;
    int menu_quad_qags;
    int menu_quad_qagi;
    int menu_quad_qawc;
    int menu_quad_qawf_sin;
    int menu_quad_qawf_cos;
    int menu_quad_qawo_sin;
    int menu_quad_qawo_cos;
    int menu_quad_qaws1;
    int menu_quad_qaws2;
    int menu_quad_qaws3;
    int menu_quad_qaws4;
    int menu_quad_qagp;
    int menu_num_domain;
    int menu_set_precision;
    int menu_set_displayprecision;
    int menu_engineeringFormat;
    int menu_engineeringFormatSetup;
    int menu_functions;
    int menu_variables;
    int menu_arrays;
    int menu_macros;
    int menu_labels;
    int menu_myoptions;
    int menu_rules;
    int menu_aliases;
    int menu_structs;
    int menu_dependencies;
    int menu_gradefs;
    int menu_let_rule_packages;
/*    menu_prop;*/
    int menu_maxima_uses_internal_help;
    int menu_maxima_uses_html_help;
    int menu_maxima_uses_wxmaxima_help;
    int menu_goto_url;
    int menu_clear_var;
    int menu_clear_fun;
    int menu_kill;
    int menu_integrate;
    int menu_risch;
    int menu_laplace;
    int menu_ilt;
    int menu_continued_fraction;
    int menu_gcd;
    int menu_lcm;
    int menu_divide;
    int menu_partfrac;
    int menu_sum;
    int menu_simpsum;
    int menu_limit;
    int menu_lbfgs;
    int menu_taylor;
    int menu_powerseries;
    int menu_fourier;
    int menu_pade;
    int menu_diff;
    int menu_solve_de;
    int menu_atvalue;
    int menu_lhs;
    int menu_rhs;
    int menu_wxmaximahelp;
    int menu_maximahelp;
    int menu_example;
    int menu_apropos;
    int menu_product;
    int menu_time;
    int menu_factsimp;
    int menu_factcomb;
    int menu_realpart;
    int menu_imagpart;
    int menu_subst;
    int menu_psubst;
    int menu_ratsubst;
    int menu_fullratsubst;
    int menu_at;
    int menu_substinpart;
    int menu_opsubst;
    int menu_jumptoerror;
    int menu_math_as_1D_ASCII;
    int menu_math_as_2D_ASCII;
    int menu_math_as_graphics;
    int menu_logexpand_false;
    int menu_logexpand_true;
    int menu_logexpand_all;
    int menu_logexpand_super;
    int menu_noAutosubscript;
    int menu_defaultAutosubscript;
    int menu_alwaysAutosubscript;
    int menu_autosubscriptIndividual;
    int menu_declareAutosubscript;
    int menu_noAutosubscriptIndividual;
    int menu_roundedMatrixParens;
    int menu_squareMatrixParens;
    int menu_straightMatrixParens;
    int menu_angledMatrixParens;
    int menu_noMatrixParens;
    int menu_draw_2d;
    int menu_draw_3d;
    int menu_draw_explicit;
    int menu_draw_implicit;
    int menu_draw_parametric;
    int menu_draw_points;
    int menu_draw_fgcolor;
    int menu_draw_fillcolor;
    int menu_draw_title;
    int menu_draw_key;
    int menu_draw_grid;
    int menu_draw_axis;
    int menu_draw_accuracy;
    int menu_draw_contour;
    int menu_license;
    int menu_changelog;
    int button_factor_id;
    int button_solve;
    int button_solve_ode;
    int button_limit;
    int button_taylor;
    int button_expand;
    int button_ratsimp;
    int button_radcan;
    int button_trigsimp;
    int button_trigexpand;
    int button_trigreduce;
    int button_trigrat;
    int button_integrate;
    int button_diff;
    int button_sum;
    int button_product;
    int button_button_constant;
    int button_factor;
    int button_subst;
    int button_plot2;
    int button_plot3;
    int button_rectform;
    int button_map;
    int gp_plot2;
    int gp_plot3;
    int menu_animationautostart;
    int menu_animationframerate;
    int menu_display;
    int menu_soft_restart;
    int menu_plot_format;
    int menu_build_info;
    int menu_bug_report;
    int menu_add_path;
    int menu_evaluate_all_visible;
    int menu_evaluate_all;
    int menu_show_tip;
    int menu_copy_matlab_from_worksheet;
    int menu_copy_tex_from_worksheet;
    int menu_copy_text_from_worksheet;
    int menu_logcontract;
    int menu_logexpand;
    int menu_to_fact;
    int menu_to_gamma;
    int menu_texform;
    int menu_debugmode;
    int menu_debugmode_off;
    int menu_debugmode_lisp;
    int menu_debugmode_all;

    // The programming menu
    int menu_for;
    int menu_while;
    int menu_block;
    int menu_block_noLocal;
    int menu_local;
    int menu_return;
    int menu_trace;
    int menu_lambda;
    int menu_quotequote;
    int menu_quote;
    int menu_quoteblock;
    int menu_def_fun;
    int menu_def_macro;
    int menu_def_variable;
    int menu_compile;
    int menu_paramType;
    int menu_structdef;
    int menu_structnew;
    int menu_structuse;
    int menu_saveLisp;
    int menu_loadLisp;
    int menu_maximatostring;
    int menu_stringtomaxima;
    
    int button_enter;
    int menu_zoom_80;
    /* Instead of menu_zoom_100 we use the standard int_ZOOM_100; which displays an icon in the menu (currently Unix only) */
    int menu_zoom_120;
    int menu_zoom_150;
    int menu_zoom_200;
    int menu_zoom_300;
    int menu_copy_as_bitmap;
    int menu_copy_as_svg;
    int menu_save_as_svg;
    int menu_copy_as_emf;
    int menu_save_as_emf;
    int menu_copy_as_rtf;
    int menu_copy_to_file;
    int menu_export_html;
    int menu_change_var;
    int menu_change_var_evaluate;
    int menu_nouns;
    int menu_evaluate;
    int menu_convert_to_code;
    int menu_add_comment;
    int menu_convert_to_comment;
    int menu_add_subsubsection;
    int menu_add_heading5;
    int menu_add_heading6;
    int menu_convert_to_subsubsection;
    int menu_convert_to_heading5;
    int menu_convert_to_heading6;
    int menu_add_subsection;
    int menu_convert_to_subsection;
    int menu_add_section;
    int menu_convert_to_section;
    int menu_add_title;
    int menu_convert_to_title;
    int menu_add_pagebreak;
    int menu_fold_all_cells;
    int menu_unfold_all_cells;
    int menu_insert_input;
    int menu_insert_previous_input;
    int menu_insert_previous_output;
    int menu_autocomplete;
    int menu_autocomplete_templates;
    int menu_paste_input;
    int menu_fullscreen;
    int menu_remove_output;
    int menu_list_create_from_elements;
    int menu_list_create_from_rule;
    int menu_list_create_from_list;
    int menu_list_create_from_args;
    int menu_list_list2matrix;
    int menu_list_matrix2list;
    int menu_list_actual_values_storage;
    int menu_list_sort;
    int menu_list_remove_duplicates;
    int menu_list_length;
    int menu_list_push;
    int menu_list_pop;
    int menu_list_reverse;
    int menu_list_first;
    int menu_list_last;
    int menu_list_lastn;
    int menu_list_rest;
    int menu_list_restN;
    int menu_list_nth;
    int menu_list_map;
    int menu_list_use_actual_values;
    int menu_list_extract_value;
    int menu_list_as_function_arguments;
    int menu_list_do_for_each_element;
    int menu_list_remove_element;
    int menu_list_append_item_start;
    int menu_list_append_item_end;
    int menu_list_append_list;
    int menu_list_interleave;
    int menu_recent_packages;
    int menu_recent_package_0;
    int menu_recent_package_1;
    int menu_recent_package_2;
    int menu_recent_package_3;
    int menu_recent_package_4;
    int menu_recent_package_5;
    int menu_recent_package_6;
    int menu_recent_package_7;
    int menu_recent_package_8;
    int menu_recent_package_9;
    int menu_recent_package_10;
    int menu_recent_package_11;
    int menu_recent_package_12;
    int menu_recent_package_13;
    int menu_recent_package_14;
    int menu_recent_package_15;
    int menu_recent_package_16;
    int menu_recent_package_17;
    int menu_recent_package_18;
    int menu_recent_package_19;
    int menu_recent_package_20;
    int menu_recent_package_21;
    int menu_recent_package_22;
    int menu_recent_package_23;
    int menu_recent_package_24;
    int menu_recent_package_25;
    int menu_recent_package_26;
    int menu_recent_package_27;
    int menu_recent_package_28;
    int menu_recent_package_29;
    int menu_recent_documents;
    int menu_recent_document_0;
    int menu_recent_document_1;
    int menu_recent_document_2;
    int menu_recent_document_3;
    int menu_recent_document_4;
    int menu_recent_document_5;
    int menu_recent_document_6;
    int menu_recent_document_7;
    int menu_recent_document_8;
    int menu_recent_document_9;
    int menu_recent_document_10;
    int menu_recent_document_11;
    int menu_recent_document_12;
    int menu_recent_document_13;
    int menu_recent_document_14;
    int menu_recent_document_15;
    int menu_recent_document_16;
    int menu_recent_document_17;
    int menu_recent_document_18;
    int menu_recent_document_19;
    int menu_recent_document_20;
    int menu_recent_document_21;
    int menu_recent_document_22;
    int menu_recent_document_23;
    int menu_recent_document_24;
    int menu_recent_document_25;
    int menu_recent_document_26;
    int menu_recent_document_27;
    int menu_recent_document_28;
    int menu_recent_document_29;
    int menu_recent_document_separator;
    int menu_unsaved_document_0;
    int menu_unsaved_document_1;
    int menu_unsaved_document_2;
    int menu_unsaved_document_3;
    int menu_unsaved_document_4;
    int menu_unsaved_document_5;
    int menu_unsaved_document_6;
    int menu_unsaved_document_7;
    int menu_unsaved_document_8;
    int menu_unsaved_document_9;
    int menu_unsaved_document_10;
    int menu_unsaved_document_11;
    int menu_unsaved_document_12;
    int menu_unsaved_document_13;
    int menu_unsaved_document_14;
    int menu_unsaved_document_15;
    int menu_unsaved_document_16;
    int menu_unsaved_document_17;
    int menu_unsaved_document_18;
    int menu_unsaved_document_19;
    int menu_unsaved_document_20;
    int menu_unsaved_document_21;
    int menu_unsaved_document_22;
    int menu_unsaved_document_23;
    int menu_unsaved_document_24;
    int menu_unsaved_document_25;
    int menu_unsaved_document_26;
    int menu_unsaved_document_27;
    int menu_unsaved_document_28;
    int menu_unsaved_document_29;
    int menu_construct_fraction;
    int menu_insert_image;
    int menu_stats_mean;
    int menu_stats_median;
    int menu_stats_var;
    int menu_stats_dev;
    int menu_stats_tt1;
    int menu_stats_tt2;
    int menu_stats_tnorm;
    int menu_stats_linreg;
    int menu_stats_lsquares;
    int menu_stats_histogram;
    int menu_stats_scatterplot;
    int menu_stats_barsplot;
    int menu_stats_piechart;
    int menu_stats_boxplot;
    int menu_stats_readm;
    int menu_stats_enterm;
    int menu_stats_subsample;
    int menu_format_code;
    int menu_format_text;
    int menu_format_heading6;
    int menu_format_heading5;
    int menu_format_subsubsection;
    int menu_format_subsection;
    int menu_format_section;
    int menu_format_title;
    int menu_format_image;
    int menu_format_pagebreak;
    int menu_help_tutorials;
    int menu_help_tutorials_start; //! Start of bundled tutorials
    int menu_help_solving;
    int menu_help_diffequations;
    int menu_help_numberformats;
    int menu_help_tolerances;
    int menu_help_listaccess;
    int menu_help_memoizing;
    int menu_help_3d;
    int menu_help_varnames;
    int menu_help_fittingData;
    int menu_help_tutorials_end; //! End of bundled tutorials
    int menu_show_toolbar;
    int menu_history_previous;
    int menu_history_next;
    int menu_check_updates;
    int gentran_load;
    int gentran_lang_c;
    int gentran_lang_fortran;
    int gentran_lang_ratfor;
    int gentran_to_stdout;
    int gentran_to_file;
    int socket_client_id;
    int socket_server_id;
    int gnuplot_query_terminals_id;
    int menu_additionalSymbols;
    int enable_unicodePane;
    int menu_showLatinGreekLookalikes;
    int menu_showGreekMu;
    int menu_invertWorksheetBackground;
  /*! @}
   */

  /*! Update the recent documents list

    Copies the string array containing the list of recent documents to the
    recent documents menu.
  */
  void UpdateRecentDocuments();
  
  /*! true, if a Pane is currently enabled

    \param id The event that toggles the visibility of the pane that is
    to be queried
  */
  bool IsPaneDisplayed(Event id);

  /*! Show or hide a sidebar
    
    \param id The type of the sidebar to show/hide
    \param show 
    - true: show the sidebar
    - false: hide it
  */
  void ShowPane(Event id, bool show = true);

  //! Adds a command to the list  of recently used maxima commands
  void AddToHistory(const wxString &cmd)
    { m_history->AddToHistory(cmd); }

  enum ToolbarStatus
  {
    wait_for_start,
    process_wont_start,
    sending,
    waiting,
    waitingForPrompt,
    waitingForAuth,
    calculating,
    parsing,
    transferring,
    userinput,
    disconnected
  };

  /*! Inform the user about the length of the evaluation queue.

   */
  void EvaluationQueueLength(int length, int numberOfCommands = -1);

  /*! Set the status according to if maxima is calculating 

    \param status
    - true:  Maxima is calculating
    - false: Maxima is waiting for input
  */
  void StatusMaximaBusy(ToolbarStatus status){m_StatusMaximaBusy_next = status;}
  void UpdateStatusMaximaBusy();

  //! True=Maxima is currently busy.
  ToolbarStatus m_StatusMaximaBusy;

  ToolbarStatus m_StatusMaximaBusy_next;

  //! Set the status to "Maxima is saving"
  void StatusSaveStart();

  //! Set the status to "Maxima has finished saving"
  void StatusSaveFinished();

  //! Set the status to "Saving has failed"
  void StatusSaveFailed();

  //! Set the status to "Maxima is exporting"
  void StatusExportStart();

  //! Set the status to "Maxima has finished exporting"
  void StatusExportFinished();

  //! Set the status to "Exporting has failed"
  void StatusExportFailed();

protected:
  Configuration m_configuration;
  //! How many bytes did maxima send us until now?
  long m_bytesFromMaxima;
  //! The process id of maxima. Is determined by ReadFirstPrompt.
  long m_pid;
  //! The last name GetTempAutosavefileName() has returned.
  wxString m_tempfileName;
  //! Issued if a notification is closed.
  void OnNotificationClose(wxCommandEvent WXUNUSED(&event));
  //! The status bar
  StatusBar *m_statusBar;
  //! The menu bar
  MainMenuBar *m_MenuBar;
  //! The "view" menu
  wxMenu *m_viewMenu;
  //! The gentran menu
  wxMenu *m_gentranMenu;
  //! The subst submenu
  wxMenu *m_subst_Sub;
  //! The logexpand submenu
  wxMenu * m_logexpand_Sub;
  //! The file menu.
  wxMenu *m_FileMenu;
  //! The edit menu.
  wxMenu *m_EditMenu;
  //! The cell menu.
  wxMenu *m_CellMenu;
  //! The zoom submenu
  wxMenu *m_Edit_Zoom_Sub;
  //! The panes submenu
  wxMenu *m_Maxima_Panes_Sub;
  //! The equations menu.
  wxMenu *m_EquationsMenu;
  //! The maxima menu.
  wxMenu *m_MaximaMenu;
  //! The matrix menu.
  wxMenu *m_matrix_menu;
  //! The simplify menu
  wxMenu *m_SimplifyMenu;
  //! The factorials and gamma submenu
  wxMenu *m_Simplify_Gamma_Sub;
  //! Contains the menu for the debugger trigger settingxc
  wxMenu *m_debugTypeMenu;
  //! The trigonometric submenu
  wxMenu *m_Simplify_Trig_Sub;
  //! The complex submenu
  wxMenu *m_Simplify_Complex_Sub;
  //! The calculus menu
  wxMenu *m_CalculusMenu;
  
  //! The plot menu
  wxMenu *m_PlotMenu;
  //! The list menu
  wxMenu *m_listMenu;
  //! The numeric menu
  wxMenu *m_NumericMenu;
  //! The help menu
  wxMenu *m_HelpMenu;
  //! Remove an eventual temporary autosave file.
  void RemoveTempAutosavefile();
  //! Re-read the configuration.
  void ReReadConfig();  
  //! Remember a temporary autosave file name.
  void RegisterAutoSaveFile();
  /*! An instant single-window mode
   
    A last resort if https://trac.wxwidgets.org/ticket/18815 hinders one from 
    re-docking windows.
  */
  void DockAllSidebars(wxCommandEvent &ev);

  wxString wxMaximaManualLocation();
protected:
  wxLocale *m_locale;
  
private:
  //! How many bytes did maxima send us when we updated the statusbar?
  long m_bytesFromMaxima_last;
  wxTimer m_bytesReadDisplayTimer; 
  //! True=We are currently saving.
  bool m_StatusSaving;

  void SetupToolBar();

/*! 
  Create the menus.
*/
  void SetupMenu();

  wxWindow *CreateStatPane();

  wxWindow *CreateMathPane();

  wxWindow *CreateFormatPane();
  
  //! The class for the sidebar with the draw commands
  class DrawPane: public wxScrolled<wxPanel>
  {
  public:
    explicit DrawPane(wxWindow *parent, int id = wxID_ANY);
    /*! Tell the sidebar if we currently are inside a 2D or a 3D plot command
        
      \param dimensions
      - 0 = We aren't inside a plot
      - 2 = We are inside a 2D plot
      - 3 = We are inside a 3D plot
    */
    void SetDimensions(int dimensions);
    int  GetDimensions() { return m_dimensions; }
  protected:
    void OnSize(wxSizeEvent &event);
  private:
    Buttonwrapsizer *m_grid;
    wxButton *m_draw_setup2d;
    wxButton *m_draw_setup3d;
    wxButton *m_draw_explicit;
    wxButton *m_draw_implicit;
    wxButton *m_draw_parametric;
    wxButton *m_draw_points;
    wxButton *m_draw_fgcolor;
    wxButton *m_draw_fillcolor;
    wxButton *m_draw_title;
    wxButton *m_draw_key;
    wxButton *m_draw_grid;
    wxButton *m_draw_axis;
    wxButton *m_draw_contour;
    wxButton *m_draw_accuracy;
    int m_dimensions;
  };
public:
  void LeftStatusText(const wxString &text, bool saveInLog = true)
    {m_newLeftStatusText = true; m_leftStatusText = text; if(saveInLog)wxLogMessage(text);}
  void RightStatusText(const wxString &text, bool saveInLog = true)
    {m_newRightStatusText = true; m_rightStatusText = text; if(saveInLog)wxLogMessage(text);}
protected:
  ScrollingGenWizPanel *m_wizard = NULL;
  //! Are we inside a 2d or 3d draw command?
  long m_drawDimensions_last;
  //! Do we have new text to output in the Right half of the Status Bar?
  bool m_newRightStatusText;
  //! Do we have new text to output in the Left half of the Status Bar?
  bool m_newLeftStatusText;
  //! The text for the Right half of the Status Bar
  wxString m_rightStatusText;
  //! The text for the Left half of the Status Bar
  wxString m_leftStatusText;
  //! The default size for the window.
  virtual wxSize DoGetBestClientSize() const;
  //! The sidebar with the draw commands
  DrawPane *m_drawPane;
  HelpBrowser *m_helpPane;
private:
  class GreekPane : public wxScrolled<wxPanel>
  {
  public:
    GreekPane(wxWindow *parent, Configuration *configuration, Worksheet *worksheet, int ID = wxID_ANY);
  protected:
    void UpdateSymbols();
    void OnMouseRightDown(wxMouseEvent &event);
    void OnMenu(wxCommandEvent &event);
    void OnSize(wxSizeEvent &event);
  private:
    Configuration *m_configuration;
    wxSizer *m_lowercaseSizer;
    wxSizer *m_uppercaseSizer;
    Worksheet *m_worksheet;
  };

  class SymbolsPane : public wxScrolled<wxPanel>
  {
  public:
    SymbolsPane(wxWindow *parent, Configuration *configuration, Worksheet *worksheet, int ID = wxID_ANY);
    //! Update the "user symbols" portion of the symbols pane.
    void UpdateUserSymbols();
  protected:
    void OnMouseRightDown(wxMouseEvent &event);
    void OnMenu(wxCommandEvent &event);
    void OnSize(wxSizeEvent &event);
  private:
    //! A panel that shows all user-defined symbols on the symbols pane.
    wxPanel *m_userSymbols;
    //! A button per user defined symbol
    std::list<wxWindow *> m_userSymbolButtons;
    wxSizer *m_userSymbolsSizer;
    Configuration *m_configuration;
    Worksheet *m_worksheet;
  };

  wxPanel *CreateSymbolsPane();

protected:
  bool m_historyVisible;
  bool m_xmlMonitorVisible;
  SymbolsPane *m_symbolsPane;
  //! The current length of the evaluation queue of commands we still need to send to maxima
  int m_EvaluationQueueLength;
  //! Do we need to update the display showing the evaluation queue length?
  bool m_updateEvaluationQueueLengthDisplay;
  //! The number of commands left in the current of the evaluation queue item
  int m_commandsLeftInCurrentCell;

  //! Do we expect the 1st prompt from maxima to appear?
  bool m_first;

  bool ToolbarIsShown();
  //! The manager for dynamic screen layouts
  wxAuiManager m_manager;
  //! A XmlInspector-like xml monitor
  XmlInspector *m_xmlInspector;
  //! true=force an update of the status bar at the next call of StatusMaximaBusy()
  bool m_forceStatusbarUpdate;
  //! The panel the log and debug messages will appear on
  LogPane *m_logPane;
  //! The worksheet itself
  Worksheet *m_worksheet;
  //! The history pane
  History *m_history;
  RecentDocuments m_recentDocuments;
  RecentDocuments m_unsavedDocuments;
  RecentDocuments m_recentPackages;
  wxMenu *m_recentDocumentsMenu;
  wxMenu *m_recentPackagesMenu;
  wxMenu *m_autoSubscriptMenu;
  wxMenu *m_equationTypeMenuMenu;
  wxMenu *m_roundedMatrixParensMenu;
};

#endif // WXMAXIMAFRAME_H
