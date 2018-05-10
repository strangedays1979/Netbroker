#ifndef C_DBTABLE_H
#define C_DBTABLE_H

#include "DBInterface.h"
#include "IDBMethods.h"


using namespace PROPERTY;
using namespace DBAccessManager;


namespace DBLib
{

	template <class RECORD>
	class CDBFactoryRecord: public DBLib::IFactoryRecord
	{
	public:
		CDBFactoryRecord(){};
		~CDBFactoryRecord(){};

		//from IFactoryRecord
		virtual DBLib::IRecord* GetNewRecord() { return new RECORD(); }
	};



	template <class RECORD>
	class   CDBTable: public IDBTable
	{
	public:

		/****************************************************************************************
		COSTRUTTORE TABELLA 
		****************************************************************************************/
		CDBTable(char *tableName)
		{
			strcpy(this->TableName, tableName);
			this->NumRows = 0;
			this->_dbManager = IDBMethodsFactory::GetDBMethods();
			this->Righe = new IRighe;
			numInsertedElement = -1;
		}
		CDBTable(char *tableName, char* _connection)
		{
			strcpy_s(this->TableName, tableName);
			this->NumRows = 0;
			this->_dbManager = IDBMethodsFactory::GetDBMethods();	
			this->_dbManager->SetConnection(_connection);
			this->Righe = new IRighe;
			numInsertedElement = -1;
		}

		~CDBTable(){};

		//from IDBTable
		virtual IFactoryRecord* GetFactoryRecord(){return &this->RecFactory;};

		//from ITable
		virtual void	SetTableName(const char* tableName){ strcpy_s(this->TableName, tableName); };
		virtual void	GetTableName(char* tableName){ strcpy_s(tableName, sizeof(this->TableName), this->TableName); };

		virtual DebDataManagement::ROW_ID	InsertItem(PROPERTY::PropertySet* row)
		{
			if (this->InsertRow((IRecord*)row))
			{
				return ((IRecord*)row)->ID_REC;
				
			}
			return 0;
		};

		virtual bool	UpdateItem(PROPERTY::PropertySet* row, DebDataManagement::ROW_ID idRow)
		{
			return (this->UpdateRow((IRecord*)row, (long)idRow) );
		};

		virtual bool	DeleteItem(DebDataManagement::ROW_ID idRow)
		{
			PROPERTY::PropertySet* element = this->GetItem(idRow);
			if (element != NULL)
			{
				return DeleteRow((IRecord*)element);
			}
			return false;

		};

		virtual PROPERTY::PropertySet* GetItem(DebDataManagement::ROW_ID idRow)
		{
			return ((PROPERTY::PropertySet*)this->GetRow((long)idRow));
		};

		
		
		/****************************************************************************************
		PRENDE LE RIGHE DELLA SORTIE TABLE
		****************************************************************************************/
		virtual  void GetAllRows(IRighe* _rows)
		{
			this->GetRows(_rows, &this->RecFactory, "1=1");
			return;
		}
		
		virtual  void GetRows(IRighe* _rows, char* where_cond)
		{
			this->GetRows(_rows, &this->RecFactory, where_cond);
			return;
		}
		virtual	bool Save()
		{

			char err[1024];
			RECORD_ROWS::iterator _iter;
			
			for (_iter = TableRows.begin(); _iter != TableRows.end(); _iter++)
			{
				IRecord * _rec = NULL;
				_rec = _iter->second;
				if (_rec->RecordStatus != ORIGINAL)
				{
					(this->Righe)->push_back(_rec);
				}
			}
			if (this->_dbManager->Save(this, err))
			{
			//	this->lastInsertedId = this->_dbManager->lastChangeId;
				return true;
			}
			return false;
		}

		/****************************************************************************************
		AGGIORNA UNA RIGA DELLA  TABLE	
		****************************************************************************************/	
		virtual bool UpdateRow(IRecord *_row)
		{
			RECORD_ROWS::iterator _iter;
			_iter = this->TableRows.find(_row->ID_REC);

			if (_iter !=  this->TableRows.end() )
			{
				this->TableRows.erase(_iter);
				RECORD* _key = (RECORD*)_row;
				_key->RecordStatus = MODIFIED;
				RECORD_ROW updated(_key->ID_REC, _key); 
				this->TableRows.insert(updated);
				return true;
			}
	
			return false;
		}
		
		//RECORD_FACTORY RecFactory;
		CDBFactoryRecord<RECORD> RecFactory;
		int lastInsertedId;


	private:
		typedef std::map<long, RECORD*> RECORD_ROWS;
		typedef std::pair<long, RECORD*> RECORD_ROW;
		RECORD_ROWS TableRows;	
		/****************************************************************************************
		INSERISCE UNA RIGA DELLA SORTIE TABLE DI TIPO <CLASS T>
		****************************************************************************************/
		virtual void	GetItems(DebDataManagement::PropertiesList & properties){ return;};
		void GetRows(IRighe *_rows, IFactoryRecord *_rec, char *_whereCondition)
		{
			// variabili per l'esecuzione di GetRows		
			char _error[255];
			long _index = 0;
			char _condition[255];
			strcpy_s(_condition, _whereCondition);
			//get Righe
			this->_dbManager->GetRows(_rows, this->TableName, _rec, _condition, _error);

			IRighe::iterator currIter; 
			RECORD* elemento;
			//ciclo per la lettura delle righe
			for (currIter = _rows->begin(); currIter != _rows->end(); currIter++)
			{	
				RECORD_ROW _key;
				((RECORD*)(*currIter))->RecordStatus = ORIGINAL;
				_key.first = ((RECORD*)(*currIter))->ID_REC;
				_key.second	= (RECORD*)(*currIter);
				this->TableRows.insert(_key);
				this->NumRows = this->NumRows + 1;	
			}			 			
			return;
		}
		
		virtual bool InsertRow(IRecord *_row)
		{
			_row->RecordStatus = tagREC_STATUS::INSERTED;;
			RECORD_ROW _key;
			_key.first = numInsertedElement--;
			_key.second	= (RECORD*)_row;
			this->TableRows.insert(_key);
			this->NumRows = this->NumRows + 1;
			return true;
		}
		
		/****************************************************************************************
		CANCELLA UNA RIGA DELLA SORTIE TABLE DI TIPO <CLASS T>
		****************************************************************************************/		
		virtual bool DeleteRow(IRecord * _row)
		{
			_row->RecordStatus = DELETED;	
			return true;
		}
		
		/****************************************************************************************
		AGGIORNA UNA RIGA DELLA SORTIE TABLE DI TIPO <CLASS T>
		****************************************************************************************/
		virtual bool UpdateRow(IRecord *_row, long index)
		{
			RECORD_ROWS::iterator _iter;
			_iter = this->TableRows.find(index);

			RECORD* _new = (RECORD*)_row;
			RECORD* _key = (_iter->second);

			_key = _new;

			return true;
		}
		
		
		/****************************************************************************************
		PRENDE UNA RIGA DELLA SORTIE TABLE DI TIPO <CLASS T>
		****************************************************************************************/
		virtual IRecord* GetRow(long index)
		{
			RECORD_ROWS::iterator _iter;
			_iter = this->TableRows.find(index);

			IRecord * _rec = NULL;

			if (_iter != this->TableRows.end())
			{
				_rec = _iter->second;
			}
			else
			{
				long ind; //eliminare index?
				char error[256];
				char whereCondition[MAX_QUERY];
				sprintf(whereCondition, " %s = %d ", RECORD::RecordKey(), index);
				_rec = this->_dbManager->GetRow(&this->RecFactory, this->TableName, whereCondition, error); 
				if(_rec != NULL)
				{
					RECORD_ROW _key;
					_rec->RecordStatus = ORIGINAL;
					_key.first = _rec->ID_REC;
					_key.second	= (RECORD*)_rec;
					this->TableRows.insert(_key);
					this->NumRows = this->NumRows + 1;
					
				}
				else
				{
					return NULL;
				}
			}

			return (IRecord*)_rec;
		}


		IDBMethods* _dbManager;
		int numInsertedElement;
		
	};
}
#endif