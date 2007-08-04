//New construction system

struct CBPlan{ // This is not used for factories building units.
	uint id;
	set<int> builders; // the builder
	int subject; // the thing being built
	bool started; // has construction started?
	float3 pos; // where is the unit being built? Used to tie the new unit to the plan
	const UnitDef* ud;
	float radius;
	bool inFactory;
};

class SkyWrite{
public:
	SkyWrite(IAICallback* cb);
	virtual ~SkyWrite();
	void Write(string Text, float3 loc, float Height=25, float Width=20, int Duration= 500, float Red=1.0f, float Green=1.0f, float Blue=1.0f, float Alpha=1.0f);
	IAICallback *GS;
};

class CManufacturer : public IModule {
public:
	CManufacturer(Global* GL);
	virtual ~CManufacturer();
	bool Init(boost::shared_ptr<IModule> me);
	void RecieveMessage(CMessage &message);
	void UnitCreated(int uid);
	void UnitFinished(int uid);
	void UnitIdle(int uid);
	void UnitMoveFailed(int uid);
	void UnitDestroyed(int uid);
	void Update();
	void EnemyDestroyed(int eid);
	float3 GetBuildPos(int builder, const UnitDef* target, const UnitDef* builderdef, float3 unitpos);
	//float3 GetBuildPlacement(int unit,float3 unitpos,const UnitDef* builder,const UnitDef* ud,int spacing);
	//bool TaskCycle(CBuilder* i);
	//bool CBuild(string name, int unit, int spacing);

	int GetSpacing(const UnitDef* ud);

	//bool LoadTaskList(CBuilder* ui);
	map<string,btype> types;
	map<btype,string> typenames;
	btype GetTaskType(string s); // retrieves the associated tasktype
	string GetTaskName(btype type); // retrieves the associated taskname
	void RegisterTaskPair(string name, btype type); // registers this pair so it can be logged and used in the tasklists
	void RegisterTaskTypes();
	string GetBuild(int uid, string tag, bool efficiency=true);
	bool CanBuild(int uid,const UnitDef* ud, string name);
	deque<CBPlan>* BPlans;
	void WipePlansForBuilder(int unit);
	int WhatIsUnitBuilding(int builder);
	bool UnitTargetStartedBuilding(int builder);
	deque<CBPlan>::iterator OverlappingPlans(float3 pos,const UnitDef* ud);

	uint getplans();
	void AddPlan();
	void RemovePlan();
	float getRranges(string unit);
	float getexclusion(string unit);
private:
	map<int,bool> factorytechlevels;
	map<int,vector<float3> > techfactorypositions;
	
	//map<int,CBuilder> builders;
	Global* G;
	bool initialized;
};