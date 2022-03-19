using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

public class SaveMap : MonoBehaviour
{
    void Start()
    {
        Debug.Log("Start Writing\r\n");
        FileStream file = new FileStream(Application.dataPath + "/MapData.tmap", FileMode.Create);
        StreamWriter writer = new StreamWriter(file);
        for (int i = 0; i < transform.childCount; ++i)
        {
            GameObject obj = transform.GetChild(i).gameObject;

            //메쉬 이름
            string name = obj.GetComponentInChildren<MeshFilter>().name;

            //좌표
            Vector3 pos = obj.transform.position;

            //쿼터니언 회전각
            Quaternion orientation = obj.transform.rotation;
            writer.Write(name + ".obj " + pos.x.ToString() + " " + pos.y.ToString() + " " + pos.z.ToString() +
                " " + orientation.x.ToString() + " " + orientation.y.ToString() + " " + orientation.z.ToString() + " " + orientation.w.ToString() + "\r\n");

            Debug.Log("Successfully Write " + name + "\r\n");
        }
        writer.Flush();
        writer.Close();
    }

    void Update()
    {
        
    }
}
