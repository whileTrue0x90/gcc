
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_JFileChooser__
#define __javax_swing_JFileChooser__

#pragma interface

#include <javax/swing/JComponent.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Component;
      namespace event
      {
          class ActionListener;
      }
    }
  }
  namespace javax
  {
    namespace accessibility
    {
        class AccessibleContext;
    }
    namespace swing
    {
        class Icon;
        class JComponent;
        class JDialog;
        class JFileChooser;
      namespace filechooser
      {
          class FileFilter;
          class FileSystemView;
          class FileView;
      }
      namespace plaf
      {
          class FileChooserUI;
      }
    }
  }
}

class javax::swing::JFileChooser : public ::javax::swing::JComponent
{

public:
  JFileChooser();
  JFileChooser(::java::lang::String *);
  JFileChooser(::java::lang::String *, ::javax::swing::filechooser::FileSystemView *);
  JFileChooser(::java::io::File *);
  JFileChooser(::javax::swing::filechooser::FileSystemView *);
  JFileChooser(::java::io::File *, ::javax::swing::filechooser::FileSystemView *);
public: // actually protected
  virtual void setup(::javax::swing::filechooser::FileSystemView *);
public:
  virtual void setDragEnabled(jboolean);
  virtual jboolean getDragEnabled();
  virtual ::java::io::File * getSelectedFile();
  virtual void setSelectedFile(::java::io::File *);
  virtual JArray< ::java::io::File * > * getSelectedFiles();
  virtual void setSelectedFiles(JArray< ::java::io::File * > *);
  virtual ::java::io::File * getCurrentDirectory();
  virtual void setCurrentDirectory(::java::io::File *);
  virtual void changeToParentDirectory();
  virtual void rescanCurrentDirectory();
  virtual void ensureFileIsVisible(::java::io::File *);
  virtual jint showOpenDialog(::java::awt::Component *);
  virtual jint showSaveDialog(::java::awt::Component *);
  virtual jint showDialog(::java::awt::Component *, ::java::lang::String *);
public: // actually protected
  virtual ::javax::swing::JDialog * createDialog(::java::awt::Component *);
public:
  virtual jboolean getControlButtonsAreShown();
  virtual void setControlButtonsAreShown(jboolean);
  virtual jint getDialogType();
  virtual void setDialogType(jint);
  virtual void setDialogTitle(::java::lang::String *);
  virtual ::java::lang::String * getDialogTitle();
  virtual void setApproveButtonToolTipText(::java::lang::String *);
  virtual ::java::lang::String * getApproveButtonToolTipText();
  virtual jint getApproveButtonMnemonic();
  virtual void setApproveButtonMnemonic(jint);
  virtual void setApproveButtonMnemonic(jchar);
  virtual void setApproveButtonText(::java::lang::String *);
  virtual ::java::lang::String * getApproveButtonText();
  virtual JArray< ::javax::swing::filechooser::FileFilter * > * getChoosableFileFilters();
  virtual void addChoosableFileFilter(::javax::swing::filechooser::FileFilter *);
  virtual jboolean removeChoosableFileFilter(::javax::swing::filechooser::FileFilter *);
  virtual void resetChoosableFileFilters();
  virtual ::javax::swing::filechooser::FileFilter * getAcceptAllFileFilter();
  virtual jboolean isAcceptAllFileFilterUsed();
  virtual void setAcceptAllFileFilterUsed(jboolean);
  virtual ::javax::swing::JComponent * getAccessory();
  virtual void setAccessory(::javax::swing::JComponent *);
  virtual void setFileSelectionMode(jint);
  virtual jint getFileSelectionMode();
  virtual jboolean isFileSelectionEnabled();
  virtual jboolean isDirectorySelectionEnabled();
  virtual void setMultiSelectionEnabled(jboolean);
  virtual jboolean isMultiSelectionEnabled();
  virtual jboolean isFileHidingEnabled();
  virtual void setFileHidingEnabled(jboolean);
  virtual void setFileFilter(::javax::swing::filechooser::FileFilter *);
  virtual ::javax::swing::filechooser::FileFilter * getFileFilter();
  virtual void setFileView(::javax::swing::filechooser::FileView *);
  virtual ::javax::swing::filechooser::FileView * getFileView();
  virtual ::java::lang::String * getName(::java::io::File *);
  virtual ::java::lang::String * getDescription(::java::io::File *);
  virtual ::java::lang::String * getTypeDescription(::java::io::File *);
  virtual ::javax::swing::Icon * getIcon(::java::io::File *);
  virtual jboolean isTraversable(::java::io::File *);
  virtual jboolean accept(::java::io::File *);
  virtual void setFileSystemView(::javax::swing::filechooser::FileSystemView *);
  virtual ::javax::swing::filechooser::FileSystemView * getFileSystemView();
  virtual void approveSelection();
  virtual void cancelSelection();
  virtual void addActionListener(::java::awt::event::ActionListener *);
  virtual void removeActionListener(::java::awt::event::ActionListener *);
  virtual JArray< ::java::awt::event::ActionListener * > * getActionListeners();
public: // actually protected
  virtual void fireActionPerformed(::java::lang::String *);
public:
  virtual void updateUI();
  virtual ::java::lang::String * getUIClassID();
  virtual ::javax::swing::plaf::FileChooserUI * getUI();
public: // actually protected
  virtual ::java::lang::String * paramString();
public:
  virtual ::javax::accessibility::AccessibleContext * getAccessibleContext();
private:
  static const jlong serialVersionUID = 3162921138695327837LL;
public:
  static const jint OPEN_DIALOG = 0;
  static const jint SAVE_DIALOG = 1;
  static const jint CUSTOM_DIALOG = 2;
  static const jint CANCEL_OPTION = 1;
  static const jint APPROVE_OPTION = 0;
  static const jint ERROR_OPTION = -1;
  static const jint FILES_ONLY = 0;
  static const jint DIRECTORIES_ONLY = 1;
  static const jint FILES_AND_DIRECTORIES = 2;
  static ::java::lang::String * CANCEL_SELECTION;
  static ::java::lang::String * APPROVE_SELECTION;
  static ::java::lang::String * APPROVE_BUTTON_TEXT_CHANGED_PROPERTY;
  static ::java::lang::String * APPROVE_BUTTON_TOOL_TIP_TEXT_CHANGED_PROPERTY;
  static ::java::lang::String * APPROVE_BUTTON_MNEMONIC_CHANGED_PROPERTY;
  static ::java::lang::String * CONTROL_BUTTONS_ARE_SHOWN_CHANGED_PROPERTY;
  static ::java::lang::String * DIRECTORY_CHANGED_PROPERTY;
  static ::java::lang::String * SELECTED_FILE_CHANGED_PROPERTY;
  static ::java::lang::String * SELECTED_FILES_CHANGED_PROPERTY;
  static ::java::lang::String * MULTI_SELECTION_ENABLED_CHANGED_PROPERTY;
  static ::java::lang::String * FILE_SYSTEM_VIEW_CHANGED_PROPERTY;
  static ::java::lang::String * FILE_VIEW_CHANGED_PROPERTY;
  static ::java::lang::String * FILE_HIDING_CHANGED_PROPERTY;
  static ::java::lang::String * FILE_FILTER_CHANGED_PROPERTY;
  static ::java::lang::String * FILE_SELECTION_MODE_CHANGED_PROPERTY;
  static ::java::lang::String * ACCESSORY_CHANGED_PROPERTY;
  static ::java::lang::String * ACCEPT_ALL_FILE_FILTER_USED_CHANGED_PROPERTY;
  static ::java::lang::String * DIALOG_TITLE_CHANGED_PROPERTY;
  static ::java::lang::String * DIALOG_TYPE_CHANGED_PROPERTY;
  static ::java::lang::String * CHOOSABLE_FILE_FILTER_CHANGED_PROPERTY;
public: // actually protected
  ::javax::accessibility::AccessibleContext * __attribute__((aligned(__alignof__( ::javax::swing::JComponent)))) accessibleContext;
private:
  ::javax::swing::filechooser::FileSystemView * fsv;
  ::javax::swing::JComponent * accessory;
  jint approveButtonMnemonic;
  ::java::lang::String * approveButtonText;
  ::java::lang::String * approveButtonToolTipText;
  ::java::util::ArrayList * choosableFilters;
  jboolean isAcceptAll;
  ::java::lang::String * dialogTitle;
  jint dialogType;
  jint retval;
  jboolean multiSelection;
  jboolean fileHiding;
  jint fileSelectionMode;
  ::javax::swing::filechooser::FileView * fv;
  jboolean controlButtonsShown;
  ::java::io::File * currentDir;
  ::javax::swing::filechooser::FileFilter * currentFilter;
  JArray< ::java::io::File * > * selectedFiles;
  ::java::io::File * selectedFile;
  jboolean dragEnabled;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_JFileChooser__
