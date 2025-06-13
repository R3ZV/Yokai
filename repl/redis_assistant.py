from crewai import Agent, Task, Crew, LLM
from crewai.tools import BaseTool
from pydantic import BaseModel, Field
from bs4 import BeautifulSoup
from duckduckgo_search import DDGS
from fastapi import FastAPI, Request
from typing import Type
import requests
import json
import uvicorn
from textwrap import dedent

# Define LLM
llm = LLM(
    model="groq/gemma2-9b-it", 
    api_key="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"
)

# Tool to search Redis documentation via DuckDuckGo
class RedisDocSearchInput(BaseModel):
    query: str = Field(..., description="Redis command or concept to search")

class RedisDocSearchTool(BaseTool):
    name: str = "RedisDocSearchTool"
    description: str = "Search Redis documentation via DuckDuckGo"
    args_schema: Type[BaseModel] = RedisDocSearchInput

    def _run(self, query: str) -> str:
        with DDGS() as ddgs:
            results = ddgs.text(f"{query} site:redis.io", max_results=2)
            summaries = []
            for res in results:
                summaries.append(
                    f"Title: {res['title']}\nURL: {res['href']}\nSnippet: {res['body']}\n"
                )
        return '\n'.join(summaries) or "No information found in Redis documentation."

# Tool instance
redis_tool = RedisDocSearchTool()

# Define Redis Expert Agent
redis_expert = Agent(
    role="Redis Expert",
    goal="Answer technical and conceptual questions about Redis",
    backstory="You are a highly knowledgeable Redis specialist capable of explaining Redis usage in detail.",
    tools=[redis_tool],
    llm=llm,
    allow_delegation=False,
    verbose=False,
)

redis_task = Task(
    description=  dedent("Answer this Redis-related question" 
                         "in a helpful and concise way: {question}"),
    agent=redis_expert,
    expected_output="A clear and informative response about Redis commands or concepts"
    )

crew = Crew(agents=[redis_expert], tasks=[redis_task], verbose=False)

# FastAPI app setup

app = FastAPI()

class RedisQuery(BaseModel):
    question: str

@app.get("/")
def read_root():
    return {"message": "Hello from FastAPI"}


@app.post("/ask")
async def ask(query: RedisQuery):
    answer = crew.kickoff(inputs={"question" : query})
    return {"answer": answer}

 
if __name__ == "__main__":
    uvicorn.run("redis_assistant:app", host="0.0.0.0", port=8000, reload=False)

