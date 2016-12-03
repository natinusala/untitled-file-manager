#include <iostream>
#include "FileTreeView.hpp"
#include "../util/File.hpp"

using namespace khanar;


//TODO Couper/copier avec this->clipboard et this->shouldDeleteClipboard

void FileTreeView::on_button_press(GdkEventButton* button_event)
{


  if((button_event->type == 4) && (button_event->button == 3))
  {
    menuPopup.popup(button_event->button, button_event->time);
  }
  else if
  ((button_event->type == 5) && (button_event->button == 1))
  {
    Gtk::TreeModel::iterator iter = m_TreeView.get_selection()->get_selected();
    int id = (*iter)[m_Columns.m_col_id];
    if (subFiles->at(id).isDirectory()){
        this->wind->actualiser(subFiles->at(id).getAbsolutePath());
    }else{
      subFiles->at(id).openFile();

    }

  }
}

void FileTreeView::on_create_directory()
{
  Gtk::Dialog dialog = Dialog("Créer un dossier", *this->parentWindow, true);

  dialog.add_button("Créer", RESPONSE_OK);
  dialog.add_button("Annuler", RESPONSE_CANCEL);

  Gtk::Label label = Label("Entrez le nom du nouveau dossier :");
  dialog.get_content_area()->pack_start(label);

  Gtk::TextView text = TextView();
  dialog.get_content_area()->pack_start(text);

  dialog.show_all_children();

  int result = dialog.run();

  if (result == RESPONSE_OK)
  {
    //Création du nouveau fichier
    string newFileName = text.get_buffer()->get_text();
    File newFile = File(this->f, newFileName);
    if (!newFile.exists() || (newFile.exists() && !newFile.isDirectory()))
    {
      newFile.createNewDirectory(S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      //TODO Mettre à jour (ici l'observateur ne marchera pas)
    }
    else
    {
      Gtk::MessageDialog dialog = MessageDialog(*this->parentWindow, "Erreur");
      dialog.set_secondary_text("Le dossier existe déjà.");
      dialog.run();
    }
  }
}

void FileTreeView::on_terminal()
{
  this->f->openXterm();
}

void FileTreeView::on_delete_file()
{
  Gtk::TreeModel::iterator iter = m_TreeView.get_selection()->get_selected();
  int id = (*iter)[m_Columns.m_col_id];

  File toDelete = this->subFiles->at(id);

  Gtk::MessageDialog dialog = MessageDialog(*this->parentWindow, "Etes-vous sûr ?");
  dialog.set_secondary_text("Etes-vous sûr de vouloir supprimer " + toDelete.getName() + (toDelete.isDirectory() ? " et tout ce qu'il contient" : "") + " ?");

  dialog.add_button("Non", RESPONSE_NO);
  int result = dialog.run();

  if (result == RESPONSE_OK)
  {
    toDelete.removeFile();
    //TODO Mettre à jour (l'observateur s'en charge normalement)
  }
}

void FileTreeView::on_rename()
{
  Gtk::TreeModel::iterator iter = m_TreeView.get_selection()->get_selected();
  int id = (*iter)[m_Columns.m_col_id];

  File toRename = this->subFiles->at(id);

  Gtk::Dialog dialog = Dialog("Renommer", *this->parentWindow, true);

  dialog.add_button("Renommer", RESPONSE_OK);
  dialog.add_button("Annuler", RESPONSE_CANCEL);

  Gtk::Label label = Label("Entrez le nouveau nom du fichier :");
  dialog.get_content_area()->pack_start(label);

  Gtk::TextView text = TextView();
  dialog.get_content_area()->pack_start(text);
  text.get_buffer()->set_text(toRename.getName());

  dialog.show_all_children();

  int result = dialog.run();

  if (result == RESPONSE_OK)
  {
    //Vérification que le nouveau fichier n'existe pas déjà
    string newFileName = text.get_buffer()->get_text();
    File newFile = File(this->f, newFileName);
    if (!newFile.exists())
    {
      toRename.setName(newFileName);
      //TODO Mettre à jour (l'observateur le fait normalement)
    }
    else
    {
      Gtk::MessageDialog dialog = MessageDialog(*this->parentWindow, "Erreur");
      dialog.set_secondary_text("Le fichier existe déjà.");
      dialog.run();
    }
  }
}

void FileTreeView::on_create_file()
{
  Gtk::Dialog dialog = Dialog("Créer un fichier", *this->parentWindow, true);

  dialog.add_button("Créer", RESPONSE_OK);
  dialog.add_button("Annuler", RESPONSE_CANCEL);

  Gtk::Label label = Label("Entrez le nom du nouveau fichier :");
  dialog.get_content_area()->pack_start(label);

  Gtk::TextView text = TextView();
  dialog.get_content_area()->pack_start(text);

  dialog.show_all_children();

  int result = dialog.run();

  if (result == RESPONSE_OK)
  {
    //Création du nouveau fichier
    string newFileName = text.get_buffer()->get_text();
    File newFile = File(this->f, newFileName);
    if (!newFile.exists())
    {
      newFile.createNewFile(S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      //TODO Mettre à jour (ici l'observateur ne marchera pas)
    }
    else
    {
      Gtk::MessageDialog dialog = MessageDialog(*this->parentWindow, "Erreur");
      dialog.set_secondary_text("Le fichier existe déjà.");
      dialog.run();
    }
  }
}


FileTreeView::FileTreeView(Gtk::Window*& win,khanar::Window* wind, string path)
{
  this->parentWindow = win;
  this->wind = wind;
  //Add the TreeView, inside a ScrolledWindow, with the button underneath:
  m_ScrolledWindow.add(m_TreeView);

  //Only show the scrollbars when they are necessary:
  m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  this->m_VBox.set_size_request(250,250);
  this->m_VBox.set_hexpand(true);
  this->m_VBox.set_vexpand(true);
  this->m_VBox.pack_start(m_ScrolledWindow);
  //Create the Tree model:
  m_refTreeModel = Gtk::ListStore::create(m_Columns);
  m_TreeView.set_model(m_refTreeModel);

  this->f = new File(path);
  subFiles = this->f->getSubFiles();

  Gtk::TreeModel::Row row;
  for (int i = 0; i < subFiles->size(); i++)
  {
    File f = (*subFiles)[i];
    if(!f.isHidden()){
      row = *(m_refTreeModel->append());
      row[m_Columns.m_col_id]=i;
      row[m_Columns.m_col_ico]= f.getFileType().getIcon();
      row[m_Columns.m_col_name] = f.getName();
      string size =f.getFormattedSize();
      if (size =="n/a"){
        row[m_Columns.m_col_number] = " ";
      }else{
        row[m_Columns.m_col_number] = size;
      }
    }
  }

  auto cell = Gtk::manage(new Gtk::CellRendererPixbuf);
  m_TreeView.append_column("   ", *cell);
  auto pColumn = m_TreeView.get_column(0);
  if(pColumn)
  {
    Glib::ustring str = "icon_name";
    pColumn->add_attribute(*cell,str,m_Columns.m_col_ico);
  }
  m_TreeView.append_column("Nom", m_Columns.m_col_name);
  m_TreeView.append_column("Taille", m_Columns.m_col_number);
  //Make all the columns reorderable:
  //This is not necessary, but it's nice to show the feature.
  //You can use TreeView::set_column_drag_function() to more
  //finely control column drag and drop.
  for(guint i = 0; i < 2; i++)
  {
    auto column = m_TreeView.get_column(i);
    column->set_reorderable();
  }


  auto item = Gtk::manage(new Gtk::MenuItem("Créer un nouveau fichier", true));
  item->signal_activate().connect_notify(
     sigc::mem_fun(*this, &FileTreeView::on_create_file) );
  menuPopup.append(*item);

  item = Gtk::manage(new Gtk::MenuItem("Créer un nouveau dossier", true));
  item->signal_activate().connect_notify(
     sigc::mem_fun(*this, &FileTreeView::on_create_directory) );
  menuPopup.append(*item);

  //Menu Pop up
 item = Gtk::manage(new Gtk::MenuItem("Couper", true));
 menuPopup.append(*item);

 item = Gtk::manage(new Gtk::MenuItem("Copier", true));
 menuPopup.append(*item);

 item = Gtk::manage(new Gtk::MenuItem("Coller", true));
 menuPopup.append(*item);

 item = Gtk::manage(new Gtk::MenuItem("Renommer", true));
 item->signal_activate().connect_notify(
    sigc::mem_fun(*this, &FileTreeView::on_rename) );
 menuPopup.append(*item);

 item = Gtk::manage(new Gtk::MenuItem("Supprimer", true));
 item->signal_activate().connect_notify(
    sigc::mem_fun(*this, &FileTreeView::on_delete_file) );
 menuPopup.append(*item);

 item = Gtk::manage(new Gtk::MenuItem("Ouvrir un terminal ici", true));
 item->signal_activate().connect_notify(
    sigc::mem_fun(*this, &FileTreeView::on_terminal) );
 menuPopup.append(*item);

 item = Gtk::manage(new Gtk::MenuItem("Ajouter/supprimer des favoris", true));
 menuPopup.append(*item);

 //menuPopup.accelerate(*this);
  menuPopup.show_all();
  m_TreeView.signal_button_press_event().connect_notify(sigc::mem_fun(*this, &FileTreeView::on_button_press), false);
  this->m_VBox.show_all_children();
}

FileTreeView::~FileTreeView()
{
  delete this->f;
}

Gtk::Box* FileTreeView::getVbox(){

  return &this->m_VBox;
}