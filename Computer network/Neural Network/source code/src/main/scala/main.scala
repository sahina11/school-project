 package prototype

import akka.actor._
import com.typesafe.config.ConfigFactory

object Main{
	val print = true

	def main(args: Array[String]): Unit = {
	  //"Usage: ./sbt "run-main prototype.Main server/supervisor supNumber

	  val serverPath = "akka.tcp://serverSystem@192.168.1.67:3333/user/server"


	  if (args(0) =="server") { //run server + supervisor
	  	val serverSystem = ActorSystem("serverSystem", ConfigFactory.load("server"))
	  	val server =serverSystem.actorOf(Props[Server],"server")
	  	val supervisorNumber = args(1).toInt

	  	server ! SupervisorNumber(supervisorNumber)

	  	val supervisorSystem = ActorSystem("supervisorSystem", ConfigFactory.load("supervisor"))	
	  	val supervisor = supervisorSystem.actorOf(Props[Supervisor])

	  	supervisor ! Path(serverPath)

	  }
	  else if (args(0) =="supervisor"){ //run supervisor only
	  	
	  	val supervisorSystem = ActorSystem("supervisorSystem", ConfigFactory.load("supervisor"))	
	  	val supervisor = supervisorSystem.actorOf(Props[Supervisor])

	  	if (print) println("\tsupervisor started")

	  	supervisor ! Path(serverPath)

	  }
	}
}