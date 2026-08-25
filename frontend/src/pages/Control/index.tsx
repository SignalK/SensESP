import { ButtonCard, Card } from "components/Card";
import { ToastMessage } from "components/ToastMessage";
import { AppPage } from "pages/AppPage";
import { type JSX } from "preact";
import { useEffect, useState } from "preact/hooks";
import { PageContents } from "../PageContents";
import { PageHeading } from "../PageHeading";

interface UIButtonInfo {
  name: string;
  title: string;
  mustConfirm: boolean;
}

export function ControlPage(): JSX.Element {
  return (
    <AppPage>
      <PageHeading title="Control" />
      <PageContents>
        <ControlCards />
      </PageContents>
    </AppPage>
  );
}

function ControlCards(): JSX.Element {
  const [buttons, setButtons] = useState<UIButtonInfo[] | null>(null);
  const [loadError, setLoadError] = useState<string>("");

  useEffect(() => {
    void (async () => {
      try {
        const res = await fetch("/api/buttons");
        if (!res.ok) {
          setLoadError(
            `Failed to load buttons: ${res.status} ${res.statusText}`,
          );
          return;
        }
        setButtons(await res.json());
      } catch (e) {
        setLoadError(`Failed to load buttons: ${(e as Error).message}`);
      }
    })();
  }, []);

  if (loadError) {
    return (
      <div className="alert alert-danger" role="alert">
        {loadError}
      </div>
    );
  }

  if (buttons === null) {
    return (
      <Card loading title="">
        <></>
      </Card>
    );
  }

  if (buttons.length === 0) {
    return (
      <p>No controls have been registered by this device&apos;s firmware.</p>
    );
  }

  return (
    <div className="vstack gap-4">
      {buttons.map((button) => (
        <UIButtonCard key={button.name} button={button} />
      ))}
    </div>
  );
}

interface UIButtonCardProps {
  button: UIButtonInfo;
}

function UIButtonCard({ button }: UIButtonCardProps): JSX.Element {
  const [httpErrorText, setHttpErrorText] = useState("");
  const [showSuccessToast, setShowSuccessToast] = useState(false);
  const [pending, setPending] = useState(false);

  async function handleClick(): Promise<void> {
    if (
      button.mustConfirm &&
      !window.confirm(`Are you sure you want to trigger "${button.title}"?`)
    ) {
      return;
    }

    setPending(true);
    try {
      const response = await fetch(
        `/api/buttons/${encodeURIComponent(button.name)}`,
        { method: "POST" },
      );
      if (!response.ok) {
        setHttpErrorText(`${response.status} ${response.statusText}`);
        return;
      }
      setShowSuccessToast(true);
    } catch (e) {
      setHttpErrorText(String(e));
    } finally {
      setPending(false);
    }
  }

  return (
    <>
      <ToastMessage
        color="text-bg-danger"
        show={httpErrorText !== ""}
        onHide={() => setHttpErrorText("")}
      >
        <p>Failed to trigger &quot;{button.title}&quot;:</p>
        <p>{httpErrorText}</p>
      </ToastMessage>
      <ToastMessage
        color="text-bg-success"
        show={showSuccessToast}
        onHide={() => setShowSuccessToast(false)}
      >
        <p>Triggered &quot;{button.title}&quot;.</p>
      </ToastMessage>
      <ButtonCard
        title={button.title}
        buttonText={button.title}
        buttonVariant={button.mustConfirm ? "danger" : "primary"}
        isButtonEnabled={!pending}
        onClick={() => {
          void handleClick();
        }}
      >
        <></>
      </ButtonCard>
    </>
  );
}
