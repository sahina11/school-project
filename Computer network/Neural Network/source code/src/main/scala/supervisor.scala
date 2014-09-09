package prototype


import scala.util.Random
import akka.actor._
import scala.collection.mutable.Map

class Supervisor extends Actor{
	val neuronPerNode = 100
	val neighborRatio = 0.1

	var neuronArray = new Array[ActorRef](1)

	var serverPath =""

	def receive = {
		case m:Path =>{
			serverPath = m.path
			context.actorSelection(serverPath) ! SupervisorPing(neuronPerNode)
		}
		case CreateNeuron =>{ //start signal from server to create neurons
			for (i <- 1 to neuronPerNode){
				var neuron = context.actorOf(Props[Neuron])
				neuron ! Path(serverPath)
			}
		}
		case m:WholeArray =>{
			neuronArray = m.array			
			context.children.foreach( actorRef => actorRef ! NeighborMap(mkNeighbor(neuronArray)) )
		}
		case Test =>{
			println("\tTest message!! ( "+ self.path+" )")
		}
	}

	def mkNeighbor(wholeMap:Array[ActorRef]): scala.collection.mutable.Map[ActorRef,Int] ={ //neighbor creation function
		val ranMachine = new Random()

		val scope = (wholeMap.size *neighborRatio).toInt
		val neighborMap = Map[ActorRef,Int]()

		val suffleArray = ranMachine.shuffle(0 to wholeMap.size-1).toArray

		for (i <- 0 to scope){
			neighborMap += ( wholeMap(suffleArray(i)) -> 1)
		}

		return neighborMap
	}
}

