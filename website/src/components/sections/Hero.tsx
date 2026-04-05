import type { HeroContent } from '../../types/content';
import { Button } from '../ui/Button';

interface HeroProps {
  content: HeroContent;
  large?: boolean;
}

export function Hero({ content, large = false }: HeroProps) {
  return (
    <section
      className={`flex items-center justify-center text-center ${
        large ? 'min-h-[80vh]' : 'py-20 lg:py-28'
      }`}
    >
      <div className="max-w-4xl mx-auto px-6">
        {content.subheadline && (
          <p className="text-accent text-sm font-medium tracking-wide uppercase mb-4">
            {content.subheadline}
          </p>
        )}
        <h1
          className={`text-text-primary font-bold leading-tight ${
            large
              ? 'text-4xl md:text-5xl lg:text-6xl'
              : 'text-3xl md:text-4xl lg:text-5xl'
          }`}
        >
          {content.headline}
        </h1>
        <p className="text-text-secondary text-lg max-w-2xl mx-auto mt-6 leading-relaxed">
          {content.description}
        </p>
        <div className="mt-8 flex flex-wrap items-center justify-center gap-4">
          <Button variant="primary" size="lg" href={content.primaryCta.href}>
            {content.primaryCta.label}
          </Button>
          {content.secondaryCta && (
            <Button variant="secondary" size="lg" href={content.secondaryCta.href}>
              {content.secondaryCta.label}
            </Button>
          )}
        </div>
      </div>
    </section>
  );
}
