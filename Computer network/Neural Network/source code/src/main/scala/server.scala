package prototype

import scala.collection.mutable.Set
import scala.collection.mutable.Map

import akka.actor._

class Server extends Actor{

	val supervisorSet = Set[ActorRef]()
	var supervisorNumber =0
	var supervisorCount = 0
	var neuronNumber = 0
	var neuronCount = 0
	var neighborPingCount = 0

	var neuronArray = new Array[ActorRef](1)
	var position = 0

	def receive ={
			case m:SupervisorNumber =>{ //predefined how many supervisor we are going to create
				supervisorNumber = m.n
			}
			case m:SupervisorPing =>{ //supervisor report to server, polling neuron number
				supervisorSet += sender //add sender				
				
				neuronNumber += m.neuronPerNode //update whole neuron #

				supervisorCount += 1 //update supervisor #
				if (supervisorCount == supervisorNumber) {
					
					neuronArray = new Array[ActorRef](neuronNumber)

					supervisorSet.foreach( actorRef => actorRef ! CreateNeuron)
				}

			}
			case Ping =>{ //neurons report to server, collecting ActorRef
				neuronArray(position) = sender
				position += 1

				neuronCount += 1

				if (position == neuronArray.size){ //the array is filled already

					supervisorSet.foreach (actorRef => actorRef ! WholeArray(neuronArray)) //send out whole array to each supervisor

					val observer = context.actorOf(Props[Observer],"observer")
					val neuronMap = Map[ActorRef,Int]()
					for (i <- 0 until neuronArray.size){
						neuronMap += ( neuronArray(i) -> i)
					}
					observer ! NeuronMap(neuronMap)
				}
			}
			case NeighborPing =>
				neighborPingCount += 1

				if (neighborPingCount == neuronCount){
					context.children.head ! StartFirstFire
				}
			case Test =>{
				println("\tTest message!! ( "+ self.path+" )")
			}
		
	}
}