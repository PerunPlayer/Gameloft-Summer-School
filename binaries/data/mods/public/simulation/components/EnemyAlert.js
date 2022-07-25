//function EnemyAlert() { }

//EnemyAlert.prototype.Schema =
//	"<element name='DetectionAlarm' a:help='Check if enemy entered outpost's vision range'>" +
//		"<data type='boolean'/>" +
//	"</element>"

//EnemyAlert.prototype.Init = function () {
//	this.alarm = false;
//};

////EnemyAlert.prototype.GetDetectionAlarm = function () {
////	var alarm = +this.template.DetectionAlarm;
////	return ApplyValueModificationsToEntity("EnemyAlert/DetectionAlarm", alarm, this.entity);
////};

///**
// * Called when units enter or leave range.
// */
//EnemyAlert.prototype.OnRangeUpdate = function(msg)
//{

//	var cmpAttack = Engine.QueryInterface(this.entity, IID_Attack);
//	if (!cmpAttack)
//		return;

//	// Target enemy units except non-dangerous animals.
//	if (msg.tag == this.gaiaUnitsQuery)
//	{
//		msg.added = msg.added.filter(e => {
//			let cmpUnitAI = Engine.QueryInterface(e, IID_UnitAI);
//			return cmpUnitAI && (!cmpUnitAI.IsAnimal() || cmpUnitAI.IsDangerousAnimal());
//		});
//	}
//	else if (msg.tag != this.enemyUnitsQuery)
//		return;

//	// Add new targets.
//	for (let entity of msg.added)
//		if (cmpAttack.CanAttack(entity))
//			this.targetUnits.push(entity);

//	// Outposts signals when enemy entered in range
//	var cmpIdentity = Engine.QueryInterface(this.entity, IID_Identity);
//	if (!cmpAttack)
//		this.alarm = false;

//	if (cmpAttack && !this.alarm) {
//		this.alarm = true;
//		warn("Entered");
//		warn(cmpIdentity.GetGenericName());
//		if (cmpIdentity.GetGenericName() == "Outpost") {
//			triggerflareaction(engine.gettarrainatscreenpoint(position.x, position.z));
//			//send message
//			//trigger flare
//		}
//	}

//	// Remove targets outside of vision-range.
//	for (let entity of msg.removed)
//	{
//		let index = this.targetUnits.indexOf(entity);
//		if (index > -1)
//			this.targetUnits.splice(index, 1);
//	}

//	if (this.targetUnits.length)
//		this.StartTimer();
//};

//Engine.RegisterComponentType(IID_EnemyAlert, "EnemyAlert", EnemyAlert);
