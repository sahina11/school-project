import org.apache.spark.SparkContext
import org.apache.spark.SparkContext._
import org.apache.spark.SparkConf
import org.apache.spark.mllib.classification.NaiveBayes
import org.apache.spark.mllib.linalg.Vectors
import org.apache.spark.mllib.regression.LabeledPoint

object Naive {
  def main(args: Array[String]) {
    val time_start = System.currentTimeMillis
    
    // val dataFile = "s3n://AKIAIDOQLVVLPIIK5GDA:wbAT9nuLG3FtHXa7dTW7TDag5UdnHXGGoVCmQQPi@lucas-coen241/200_MB_Test_dat_tab.tsv"
    val dataFile = "tachyon://ec2-54-85-149-164.compute-1.amazonaws.com:19998/data/1_GB_Train_and_Test_data.tsv"
    val conf = new SparkConf().setAppName("NaiveBayes Application")
    val sc = new SparkContext(conf)
    
    val rawdata = sc.textFile(dataFile)

    val classname = rawdata.map(line =>{ val x=line.split("\t"); x(0)}).distinct
    
    val name_mapping = classname.zipWithIndex().collect.toMap 

    // val reverse_mapping = name_mapping.map(_.swap) 

    val data= rawdata.map( line=>{
    	val triple = line.split("\t"); name_mapping(triple(0)).toString+","+triple(2)})

    val features_per_image:Array[Int] = data.map{ line=> val part = line.split(','); part(1).split(' ').size}.collect
    val average_feature_quant = (features_per_image.sum / features_per_image.size).toInt
    val max_feature_quant = features_per_image.max

  //   val parsedData = data.map{ line => //max version
  //     val parts = line.split(',')
  //     val feature_array = parts(1).split(' ').map(_.toDouble)
  //     val feature_quant = feature_array.size

  //     if (feature_quant == max_feature_quant){
  //     	LabeledPoint(parts(0).toDouble, Vectors.dense(feature_array))
  //     }
  //     else{
  //     	val diff= max_feature_quant - feature_quant
		// LabeledPoint(parts(0).toDouble, Vectors.dense(feature_array.union(new Array[Double](diff))))      	
  //     }
  //   }

   val parsedData = data.map{ line => //average version
      val parts = line.split(',')
      val feature_array = parts(1).split(' ').map(_.toDouble)
      val feature_quant = feature_array.size
      
      if (feature_quant > average_feature_quant){
      	LabeledPoint(parts(0).toDouble, Vectors.dense(feature_array.slice(0,average_feature_quant)))
      }
      else if (feature_quant < average_feature_quant){
      	val diff= average_feature_quant - feature_quant
		    LabeledPoint(parts(0).toDouble, Vectors.dense(feature_array.union(new Array[Double](diff))))      	
      }
      else{
      	LabeledPoint(parts(0).toDouble, Vectors.dense(feature_array))
      }
    }

    /*val parsedData = data.map{ line => //original version
      val parts = line.split(',')
      LabeledPoint(parts(0).toDouble, Vectors.dense(parts(1).split(' ').map(_.toDouble).take(features_to_make)))
    }*/

    val splits = parsedData.randomSplit(Array(0.8, 0.2), seed = 11L)
    val training = splits(0)
    val test = splits(1)

    val model = NaiveBayes.train(training, lambda = 1.0)
    
    val prediction = model.predict(test.map(_.features))

    val predictionAndLabel = prediction.zip(test.map(_.label))
    val accuracy = 1.0 * predictionAndLabel.filter(x => x._1 == x._2).count() / test.count()
    println(accuracy)
    println("Total execution time: "+ ((System.currentTimeMillis-time_start)/1000).toString+ " seconds.")
  }
}



